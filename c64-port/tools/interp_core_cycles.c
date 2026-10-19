/* SPDX-License-Identifier: MIT */
/* 6502 cycles of the compact interpreter (src/lm_interp_core.c), one NEWOPS
 * slot at a time, in llvm-mos's simulator (mos-sim, whose clock() counts
 * cycles), with the image at $7000 as on the C64. Reads slot records from
 * stdin (tools/interp_core_fuzz.c FUZZ_DUMP): the image before the slot and
 * the rope words the slot reads. The rope reads here come from that list;
 * their time is taken out and the C64 read path's own time
 * (platform/c64/rope_image_reu.c over src/overlay_reu.c, without the
 * two-cycle DMA) is measured once and printed, so a slot's C64 cost is
 * cycles + reads x that. Prints "rope <cycles>" then per slot
 * "<Z> <word at LOC> <cycles> <reads> <status>", then the words that differ
 * from the host's result (the record carries it) and MISMATCH.
 *
 * Build: mos-sim-clang -Oz -DINTERP_CYCLES_CM? (see tools/interp_core_cycles.py). */
#include <stdio.h>
#include <time.h>
#include "interp_runtime.h"
#include "overlay_runtime.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#include "rope_image.h"

bool reu_rope_read(unsigned mission, uint16_t physical, agc_word *word);
/* The REU registers as plain bytes (the program occupies $DF00 here):
 * the same absolute stores as platform/c64/overlay_reu_platform.c. */
#include "overlay_reu.h"
static volatile uint8_t reu_regs[16];
void apollo_reu_poke(uint8_t reg, uint8_t value) { reu_regs[reg] = value; }
uint8_t apollo_reu_peek(uint8_t reg) { return reu_regs[reg]; }
uint16_t apollo_reu_c64_address(const void *p) { return (uint16_t)(uintptr_t)p; }

#define READS 96u
static uint16_t nread, rp[READS];
static agc_word rv[READS];
static uint16_t used;
static long in_rope;
bool apollo_rope_read(unsigned mission, uint16_t physical, agc_word *word)
{
    long t0 = clock();
    uint16_t i;
    bool ok = false;
    (void)mission;
    for (i = 0; i < nread; ++i) if (rp[i] == physical) { *word = rv[i]; ok = true; break; }
    ++used;
    in_rope += clock() - t0;
    return ok;
}

/* (printf would not leave room for the interpreter) */
static void put_num(unsigned long v, unsigned base)
{
    char b[12];
    uint8_t n = 0;
    do b[n++] = (char)('0' + v % base); while ((v /= base) != 0);
    while (n) putchar(b[--n]);
}
static void put_str(const char *t) { while (*t) putchar(*t++); }
static int get16(uint16_t *v)
{
    int lo = getchar(), hi;
    if (lo == EOF) return 0;
    hi = getchar();
    *v = (uint16_t)(lo | (hi << 8));
    return 1;
}

int main(void)
{
    apollo_overlay_state *s = (apollo_overlay_state *)APOLLO_SHARED_ADDRESS;
    uint16_t magic, i, ie, sb;
    long t0, t1, empty, rope, call;
    agc_word w;
    /* The clock pair's own cost, and the C64 rope read path's. */
    t0 = clock(); t1 = clock(); empty = t1 - t0;
    t0 = clock();
    for (i = 0; i < 16u; ++i) (void)reu_rope_read(INTERP_CYCLES_MISSION, 2048u + i, &w);
    rope = (clock() - t0 - empty) / 16;
    /* What a list read costs outside its own timed part. */
    nread = 1; rp[0] = 0; in_rope = 0;
    t0 = clock();
    for (i = 0; i < 16u; ++i) (void)apollo_rope_read(0, 0, &w);
    call = (clock() - t0 - empty - in_rope) / 16;
    put_str("rope "); put_num((unsigned long)rope, 10); putchar('\n');
    while (get16(&magic) && magic == 0xa6c1u) {
        enum exec_status st;
        uint16_t z, word, expect, bad;
        for (i = 0; i < 2048u; ++i) (void)get16(&s->words[i]);
        (void)get16(&ie); (void)get16(&sb); (void)get16(&nread);
        if (nread > READS) return 1;
        for (i = 0; i < nread; ++i) { (void)get16(&rp[i]); (void)get16(&rv[i]); }
        (void)get16(&word);
        z = s->words[5];
        used = 0; in_rope = 0;
#if INTERP_CYCLES_MISSION
        {
            cm_native_io io = { 0 };
            io.superbank = sb; io.interrupt_enabled = ie;
            cm_interp_core_mct = 0; t0 = clock(); st = cm_interp_core(0, s, &io); t1 = clock();
        }
#else
        {
            p66_bias_io io = { 0 };
            io.superbank = sb; io.interrupt_enabled = ie;
            lm_interp_core_mct = 0; t0 = clock(); st = lm_interp_core(0, s, &io); t1 = clock();
        }
#endif
        put_num(z, 8); putchar(' '); put_num(word, 8); putchar(' ');
        put_num((unsigned long)(t1 - t0 - empty - in_rope - (long)used * call), 10); putchar(' ');
        put_num(used, 10); putchar(' '); put_num((unsigned)st, 10);
        for (i = 0, bad = 0; i < 2048u; ++i) {
            (void)get16(&expect);
            if (s->words[i] != expect && bad++ < 8u) {
                putchar(' '); put_num(i, 8); putchar('='); put_num(expect, 8); putchar('/'); put_num(s->words[i], 8);
            }
        }
        put_str(bad ? " MISMATCH\n" : "\n");
    }
    return 0;
}
