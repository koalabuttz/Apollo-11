/* SPDX-License-Identifier: MIT */
/* The compact interpreter on a PAL C64 in VICE (tools/interp_core_vice_test.py):
 * the flight module's build (AGC_FIXED_IMAGE, INTERP_K_OPS, INTERP_EXTENSION)
 * runs slot records from the REU and checks each result there.
 *
 * - The rope is at its flight REU address and read by DMA
 *   (platform/c64/rope_image_reu.c), as in flight.
 * - The k_ ops are the resident kernel's (k_<op>(a) is ai_<op> on the image
 *   at $7000, src/resident_kernel_body.inc).
 * - The extension's bytes are copied to $D000 with I/O banked out, as the
 *   flight loader will.
 * - The CIA 1 timer IRQ runs throughout, through a RAM vector whose entry
 *   saves $01 and banks I/O in: interrupts land while the order bodies run
 *   with I/O banked out.
 *
 * REU layout (written by the test): at 0 the record count (2 bytes); record k
 * at (k + 1) * $2400: the 2048-word image before the slot, the 2048-word
 * image after it (the host's verification interpreter), then the count and
 * (physical address, word) pairs of the rope words the slot reads (the
 * fuzz's generated program), which go into the REU rope first. The
 * interrupt-enable state is 1 and the superbank 0 (the fuzz's records).
 *
 * A run with no interrupt during the records counts as one more failure. */
#include <stdint.h>
#include "agc_image_ops.h"
#include "interp_runtime.h"
#include "overlay_reu.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#include "rope_image.h"
#ifdef INTERP_VICE_CM
#define ROPE_BASE APOLLO_ROPE_REU_CM
#else
#define ROPE_BASE APOLLO_ROPE_REU_LM
#endif

/* The resident's image ops (src/resident_kernel_body.inc). */
#define IMG ((agc_image *)APOLLO_SHARED_ADDRESS)
#define KOP(n, T) void k_##n(T a); void k_##n(T a) { ai_##n(IMG, a); }
#define KOPS(n, T) uint8_t k_##n(T a); uint8_t k_##n(T a) { return ai_##n(IMG, a); }
KOP(ca, unsigned) KOP(caf, agc_word) KOP(cs, unsigned) KOP(csf, agc_word) KOP(ad, unsigned) KOP(adf, agc_word)
KOP(maskf, agc_word) KOPS(ts, unsigned) KOP(xch, unsigned) KOP(lxch, unsigned) KOP(ads, unsigned) KOP(dxch, unsigned)
KOPS(ccs, unsigned) KOP(tc, agc_word) KOP(dcaf, const agc_word *)
uint8_t k_bzf(void);
uint8_t k_bzf(void) { return ai_bzf(IMG); }
uint8_t k_bzmf(void);
uint8_t k_bzmf(void) { return ai_bzmf(IMG); }

#define CPU_PORT (*(volatile uint8_t *)1)
#define CIA1_ICR (*(volatile uint8_t *)0xdc0du)
#define MAIL ((volatile uint8_t *)0xc000u)
/* tools/vice_run.py's mailbox: $A5, failures, checks; then the IRQ count and
 * the first failure (record, word, 6502 value, host value, status). */
enum { MAIL_DONE = 0, MAIL_FAIL = 1, MAIL_CHECKS = 3, MAIL_IRQS = 5, MAIL_FIRST = 7, MAIL_WORD = 9, MAIL_GOT = 11,
       MAIL_WANT = 13, MAIL_STATUS = 15 };
static void mail16(uint8_t at, uint16_t v) { MAIL[at] = (uint8_t)v; MAIL[at + 1] = (uint8_t)(v >> 8); }

static volatile uint16_t irqs;
/* The resident kernel's IRQ entry: save $01, bank I/O in, acknowledge. */
__attribute__((interrupt)) static void irq(void)
{
    uint8_t m = CPU_PORT;
    CPU_PORT = 0x35;
    (void)CIA1_ICR;
    ++irqs;
    CPU_PORT = m;
}
__attribute__((interrupt)) static void nmi(void) {}

extern char __interp_ext_start[], __interp_ext_end[], __interp_ext_load[];

/* Buffers above the image ($8000-$81FF; the program ends below $7000). */
#define want ((agc_word *)0x8000u)
#define pairs ((uint16_t *)0x8100u)
#define PAIRS_BYTES (2u * (2u * 96u + 1u))

int main(void)
{
    uint16_t n, k, i, j, pass = 0, fail = 0;
    uint32_t at;
    __asm__ volatile("sei");
    CPU_PORT = 0x35;   /* KERNAL and BASIC out: the vectors are RAM */
    *(volatile uint16_t *)0xfffeu = (uint16_t)(uintptr_t)irq;
    *(volatile uint16_t *)0xfffau = (uint16_t)(uintptr_t)nmi;
    CPU_PORT = 0x34;
    for (i = 0; i < (uint16_t)(__interp_ext_end - __interp_ext_start); ++i) __interp_ext_start[i] = __interp_ext_load[i];
    CPU_PORT = 0x35;
    __asm__ volatile("cli");
    (void)apollo_reu_transfer(APOLLO_REU_FETCH, &n, 0, 2);
    mail16(MAIL_FIRST, 0xffffu);
    for (k = 0; k < n; ++k) {
        enum exec_status st;
        uint8_t bad = 0;
        at = (uint32_t)(k + 1u) * 0x2400u;
        (void)apollo_reu_transfer(APOLLO_REU_FETCH, pairs, at + 8192u, PAIRS_BYTES);
        for (i = 0; i < pairs[0] && i < 96u; ++i)
            (void)apollo_reu_transfer(APOLLO_REU_STASH, &pairs[1 + 2 * i + 1], ROPE_BASE + 2u * (uint32_t)pairs[1 + 2 * i], 2u);
        (void)apollo_reu_transfer(APOLLO_REU_FETCH, IMG->words, at, 4096u);
#ifdef INTERP_VICE_CM
        { cm_native_io io = { 0 }; io.interrupt_enabled = 1; st = cm_interp_core(0, (apollo_overlay_state *)IMG, &io); }
#else
        { p66_bias_io io = { 0 }; io.interrupt_enabled = 1; st = lm_interp_core(0, (apollo_overlay_state *)IMG, &io); }
#endif
        for (i = 0; i < 2048u && !bad; i += 128u) {
            (void)apollo_reu_transfer(APOLLO_REU_FETCH, want, at + 4096u + 2u * i, 256u);
            for (j = 0; j < 128u; ++j)
                if (IMG->words[i + j] != want[j]) {
                    if (!fail) {
                        mail16(MAIL_FIRST, k); mail16(MAIL_WORD, i + j);
                        mail16(MAIL_GOT, IMG->words[i + j]); mail16(MAIL_WANT, want[j]); mail16(MAIL_STATUS, (uint16_t)st);
                    }
                    bad = 1;
                    break;
                }
        }
        if (bad) ++fail; else ++pass;
    }
    mail16(MAIL_IRQS, irqs);
    mail16(MAIL_FAIL, fail + (irqs == 0u)); mail16(MAIL_CHECKS, pass + fail + 1u);
    MAIL[MAIL_DONE] = 0xa5;
    for (;;) {}
}
