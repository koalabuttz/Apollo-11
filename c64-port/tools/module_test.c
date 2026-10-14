/* SPDX-License-Identifier: MIT */
/* The C64 module-test harness (tools/module_test.py, docs/MODULE-TEST.md):
 * the flight resident boots on the flight store (build/flight-store/<m>.reu),
 * then runs case vectors streamed from the REU through the flight modules
 * under test, loaded from the store into their slots as the resident loads
 * them, and compares each complete result by CRC-32 with the host's
 * (the Virtual AGC oracle's, or the unchanged state for a refused case).
 *
 * MT_DATA names the generated header: MT_MODULES (the module ids under
 * test), MT_IDLE_Z (a fixed-fixed point no store table holds, from which a
 * module run only loads the module and builds its table list), MT_CASES (the
 * REU offset of the case blob) and MT_IO_<kind> (the device record the
 * modules' routines take). The blob: a 16-byte header ("MTC1", case count,
 * record bytes) and one record per case: the image's 2048 words, channels
 * 0-077, superbank, IRQ enable, the stop point (AGC_TRANSFER_NONE: none), the
 * routine limit, the expected CRC-32 and the case number.
 *
 * A case runs routines from its entry point, one at a time, through the
 * table lists of the loaded modules under test. At a point none of them
 * holds, the store's resolver names the module; one under test is loaded
 * (an ABI 2 run from MT_IDLE_Z) and the point retried; any other ends the
 * case, as does the stop point or the routine limit. A case whose entry
 * no module under test holds, or whose first routine refuses, is a refused
 * case (status 1, the state must be unchanged); a routine refusing later is
 * status 2, which no host row has.
 *
 * Mailbox (tools/module_test.py): $c001-2 failures, $c003-4 cases compared, $c005
 * the first failing step (1 boot, 2 blob header, 3 module load, 4 compare),
 * $c006-7 routines run, $c008 the count of failing cases listed at $c010
 * (case numbers, 4 bytes each, up to 32), $a5 at $c000. */
#include <stdint.h>
#include "resident_flight.h"
#include MT_DATA
#if MT_IO_LM_DNTM
#include "lm_downtelemetry.h"
typedef lm_dntm_io mt_io;
#define MT_SUPERBANK(d) (d).control.superbank
#define MT_IRQ(d) (d).control.interrupt_enabled
#elif MT_IO_CM_DNTM
#include "cm_downtelemetry.h"
typedef cm_dntm_io mt_io;
#define MT_SUPERBANK(d) (d).io.superbank
#define MT_IRQ(d) (d).io.interrupt_enabled
#else
#error "tools/module_test.c: no device record for this io kind yet"
#endif

#define IMAGE ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
#define MT_CHANNELS 0100u
#define MT_HEADER 16u
typedef struct {
    agc_word channel[MT_CHANNELS];
    agc_word superbank, irq, stop, limit;
    uint32_t crc, n;
} mt_tail;
#define MT_RECORD (APOLLO_SHARED_WORDS * 2u + sizeof(mt_tail))

static volatile uint8_t *const mail = (volatile uint8_t *)0xc000;
static uint16_t failures, checks, listed;
static uint8_t first_step;
/* checks counts the cases compared; boot, the header and loads only fail. */
static void expect(uint8_t ok, uint8_t step)
{
    if (!ok) { ++failures; if (!first_step) mail[5] = first_step = step; }
}

/* One REU fetch that may span a 512 KB bank: a single DMA wraps inside its
 * bank (VICE), so a transfer is split at each boundary. */
static void fetch(void *to, uint32_t from, uint16_t bytes)
{
    uint8_t *p = to;
    while (bytes) {
        uint32_t room = 0x80000ul - (from & 0x7fffful);
        uint16_t n = room < bytes ? (uint16_t)room : bytes;
        apollo_reu_transfer(APOLLO_REU_FETCH, p, from, n);
        p += n; from += n; bytes = (uint16_t)(bytes - n);
    }
}

/* The device record from the case's channels, and back. */
static mt_io device;
static mt_tail tail;
static void put_device(void)
{
    memset(&device, 0, sizeof device);
    MT_SUPERBANK(device) = tail.superbank; MT_IRQ(device) = tail.irq;
    rk_hardware.channel11 = tail.channel[011]; rk_hardware.in_interrupt = 0;
#if MT_IO_LM_DNTM
    device.channel11 = tail.channel[011]; device.channel12 = tail.channel[012];
    device.channel13 = tail.channel[013]; device.channel14 = tail.channel[014];
    device.channel30 = tail.channel[030]; device.channel31 = tail.channel[031];
    device.channel32 = tail.channel[032]; device.channel33 = tail.channel[033];
    device.channel34 = tail.channel[034]; device.channel35 = tail.channel[035];
#else
    device.io.channel11 = tail.channel[011]; device.io.channel12 = tail.channel[012];
    device.io.channel13 = tail.channel[013]; device.io.channel14 = tail.channel[014];
    device.io.main_keys = tail.channel[015]; device.io.navigation_keys = tail.channel[016];
    device.channel30 = tail.channel[030]; device.channel31 = tail.channel[031];
    device.channel32 = tail.channel[032]; device.channel33 = tail.channel[033];
    device.channel34 = tail.channel[034]; device.channel35 = tail.channel[035];
#endif
}
static void get_device(void)
{
    tail.superbank = MT_SUPERBANK(device); tail.irq = MT_IRQ(device);
#if MT_IO_LM_DNTM
    tail.channel[011] = device.channel11; tail.channel[012] = device.channel12;
    tail.channel[013] = device.channel13; tail.channel[014] = device.channel14;
    tail.channel[030] = device.channel30; tail.channel[031] = device.channel31;
    tail.channel[032] = device.channel32; tail.channel[033] = device.channel33;
    tail.channel[034] = device.channel34; tail.channel[035] = device.channel35;
#else
    tail.channel[011] = device.io.channel11; tail.channel[012] = device.io.channel12;
    tail.channel[013] = device.io.channel13; tail.channel[014] = device.io.channel14;
    tail.channel[015] = device.io.main_keys; tail.channel[016] = device.io.navigation_keys;
    tail.channel[030] = device.channel30; tail.channel[031] = device.channel31;
    tail.channel[032] = device.channel32; tail.channel[033] = device.channel33;
    tail.channel[034] = device.channel34; tail.channel[035] = device.channel35;
#endif
}

/* CRC-32 (zlib) with a 16-entry table, as tools/lanec/c64_diag.c: every
 * image word, channels 0-077, superbank, IRQ and status, little-endian. */
static const uint32_t nibble[16] = {
    0x00000000u, 0x1db71064u, 0x3b6e20c8u, 0x26d930acu, 0x76dc4190u, 0x6b6b51f4u, 0x4db26158u, 0x5005713cu,
    0xedb88320u, 0xf00f9344u, 0xd6d6a3e8u, 0xcb61b38cu, 0x9b64c2b0u, 0x86d3d2d4u, 0xa00ae278u, 0xbdbdf21cu };
static uint32_t crc;
static void crc_word(uint16_t v)
{
    uint8_t k;
    for (k = 0; k < 2u; ++k, v >>= 8) {
        crc ^= (uint8_t)v;
        crc = nibble[crc & 15u] ^ (crc >> 4);
        crc = nibble[crc & 15u] ^ (crc >> 4);
    }
}
static uint32_t result_crc(uint16_t status)
{
    uint16_t i;
    crc = 0xffffffffu;
    for (i = 0; i < APOLLO_SHARED_WORDS; ++i) crc_word(IMAGE->words[i]);
    for (i = 0; i < MT_CHANNELS; ++i) crc_word(tail.channel[i]);
    crc_word(tail.superbank); crc_word(tail.irq); crc_word(status);
    return crc ^ 0xffffffffu;
}

static const uint16_t modules[] = { MT_MODULES };
#define MT_MODULE_COUNT (sizeof modules / sizeof modules[0])
static uint8_t under_test(uint16_t id)
{
    uint8_t i;
    for (i = 0; i < MT_MODULE_COUNT; ++i)
        if (modules[i] == id) return 1;
    return 0;
}

/* Load a module under test: an ABI 2 run from MT_IDLE_Z, which no table
 * holds, loads it and builds its table list, and returns at once. */
static uint8_t load(uint16_t id)
{
    apollo_ov2_cont at = { id, 0 }, resume;
    uint16_t value, z = IMAGE->words[5], fb = IMAGE->words[4];
    uint8_t action, ok;
    IMAGE->words[5] = MT_IDLE_Z;
    value = (uint16_t)(uintptr_t)&rk_env; rk_env.status = EXEC_OK; rk_env.io = &device;
    ok = apollo_ov2_run(&rk_runtime, 0, at, &value, &action, &resume) == APOLLO_OV2_OK &&
         action == APOLLO_OV2_DONE && rk_env.status == EXEC_UNMAPPED;
    IMAGE->words[5] = z; IMAGE->words[4] = fb;
    return ok;
}

/* The routine at the current point in a loaded module under test. */
static agc_native_program program;
static agc_routine find(void)
{
    uint8_t i;
    for (i = 0; i < APOLLO_OV2_SLOTS; ++i) {
        const uint8_t *h = rk_runtime.slot[i];
        const rk_module_tables *t;
        agc_routine call;
        if (!rk_runtime.loaded[i] || !under_test(rk_runtime.loaded[i])) continue;
        t = (const rk_module_tables *)(uintptr_t)(h[APOLLO_OV2_H_TABLES] | (uint16_t)h[APOLLO_OV2_H_TABLES + 1] << 8);
        if (!t->ready) continue;
        memcpy(&program, &rk_program, sizeof program);
        rk_module_program(&program, t);
        call = agc_transfer_resolve(&program, IMAGE, MT_SUPERBANK(device));
        if (call) return call;
    }
    return 0;
}

static uint16_t routines;
static uint16_t run_case(void)
{
    uint16_t n = 0, loc;
    apollo_ov2_cont at;
    agc_routine call;
    enum exec_status status;
    while (n < tail.limit && agc_transfer_point(&rk_program, IMAGE, MT_SUPERBANK(device)) != tail.stop) {
        call = find();
        if (!call) {
            loc = (uint16_t)(IMAGE->words[5] & 07777u);
            if (!rk_runtime.store.resolve(rk_runtime.store.user,
                                          rk_resolver_bank(IMAGE->words[4], MT_SUPERBANK(device), loc), loc, &at) ||
                !under_test(at.module))
                break;
            if (!load(at.module)) { expect(0, 3); break; }
            call = find();
            if (!call) break;
        }
        ++n; ++routines;
        status = call(&program, IMAGE, &device);
        if (status == EXEC_OUT_OF_SCOPE) return n == 1u ? 1u : 2u;
        if (status != EXEC_OK) break;
    }
    return n ? 0u : 1u;   /* an entry no module under test holds is refused */
}

int main(void)
{
    uint8_t head[MT_HEADER];
    uint16_t i, count, status;
    uint32_t at;
    for (i = 1; i < 0x10u; ++i) mail[i] = 0;
    expect(rk_boot() == 0, 1);
    if (!failures) {
        __asm__ volatile("sei");
        fetch(head, MT_CASES, MT_HEADER);
        count = (uint16_t)(head[4] | head[5] << 8);
        expect(head[0] == 'M' && head[1] == 'T' && head[2] == 'C' && head[3] == '1' &&
               (uint16_t)(head[6] | head[7] << 8) == MT_RECORD, 2);
        if (!failures)
            for (i = 0, at = MT_CASES + MT_HEADER; i < count; ++i, at += MT_RECORD) {
                fetch(IMAGE->words, at, APOLLO_SHARED_WORDS * 2u);
                fetch(&tail, at + APOLLO_SHARED_WORDS * 2u, sizeof tail);
                put_device();
                status = run_case();
                get_device();
                ++checks;
                if (result_crc(status) != tail.crc) {
                    expect(0, 4);
                    if (listed < 32u) { ((volatile uint32_t *)0xc010)[listed++] = tail.n; mail[8] = (uint8_t)listed; }
                }
            }
        __asm__ volatile("cli");
    }
    mail[6] = (uint8_t)routines; mail[7] = (uint8_t)(routines >> 8);
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
