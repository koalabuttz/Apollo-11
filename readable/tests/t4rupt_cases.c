/* SPDX-License-Identifier: MIT */
/* The readable T4RUPT core against the faithful port's, case by case
 * (make t4rupt-cases).
 *
 * Built once per mission, with the readable tables (src/binding/
 * t4rupt_routines.c) and the faithful port's under the same names with
 * faithful_ in front (the Makefile renames them). Each entry of each table
 * runs on 65,536 random states: random memory in bank 06, mostly inside
 * the interrupt; DSRUPTSW a phase count, negative (the 20 ms phase) or
 * now and then out of range; DSPCNT and the display table mostly as the
 * display programs keep them, some words waiting to be sent; NOUT, the
 * DSKY flag, IMODES33, the PRO key and RUPTREG1 random; an overflow in A
 * now and then. Where either port runs, the other must too, with the same
 * memory, io record and channels, word for word; where the readable one
 * refuses, it must leave everything as it found it. The faithful routines
 * are certified against the original program (c64-port/tests/
 * test_t4rupt_core.c), as the readable ones are by make t4rupt-cases'
 * first step. */
#include <stdio.h>
#include <string.h>
#include "t4rupt_core.h"
#if LUMINARY099
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION     "LM"
#define DSRUPTSW_AT 01313u
extern const agc_routine_table faithful_lm_t4rupt_core_routines,
    faithful_lm_t4rupt_core_hot_routines;
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {
    {"lm_t4rupt_core_routines", &lm_t4rupt_core_routines, &faithful_lm_t4rupt_core_routines},
    {"lm_t4rupt_core_hot_routines", &lm_t4rupt_core_hot_routines,
     &faithful_lm_t4rupt_core_hot_routines}};
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION     "CM"
#define DSRUPTSW_AT 01302u
extern const agc_routine_table faithful_cm_t4rupt_core_routines;
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {
    {"cm_t4rupt_core_routines", &cm_t4rupt_core_routines, &faithful_cm_t4rupt_core_routines}};
#endif

#define CASES 65536u

static uint32_t seed = 223606u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

/* A 16-bit register: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    return rnd() % 16u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t z,
                  uint16_t fb)
{
    unsigned i;
    uint16_t eb = (uint16_t)(rnd() % 8u), x;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = sign_extended(rnd());
    s->words[2] = sign_extended(rnd());
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = rnd() % 16u ? fb : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = z;
    s->words[6] = (uint16_t)(s->words[4] | eb);
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    /* DSRUPTSW: a phase count, negative (the 20 ms phase, some near -0),
     * or now and then out of range. */
    x = (uint16_t)(rnd() % 8u);
    s->words[DSRUPTSW_AT] = (uint16_t)(x < 3u    ? rnd() % 9u
                                       : x == 3u ? 0u
                                       : x == 4u ? rnd() & 077777u
                                                 : (rnd() & 037777u) | 040000u);
    if (x == 7u) s->words[DSRUPTSW_AT] |= 0377u;
    /* DSPCNT 0-012 or -0, now and then not. */
    s->words[0776] = (uint16_t)(rnd() % 12u);
    if (s->words[0776] == 013u) s->words[0776] = 077777u;
    if (rnd() % 64u == 0u) s->words[0776] = (uint16_t)(rnd() & 077777u);
    /* DSPTAB: mostly sent (positive), some waiting (negative); now and then
     * a zero. */
    for (i = 0; i < 12u; ++i) {
        x = (uint16_t)(rnd() & 077777u);
        if (x == 077777u || rnd() % 4u) x &= 037777u;
        if (x == 0u && i < 11u) x = 1u;
        if (rnd() % 512u == 0u) x = rnd() % 2u ? 0u : 077777u;
        s->words[01023u + i] = x;
    }
    /* NOUT: a few words, none, or negative. */
    s->words[01016] = rnd() % 4u ? (uint16_t)(rnd() % 12u) : (uint16_t)(rnd() & 077777u);
    /* RUPTREG1 0-7, now and then not. */
    s->words[070] = rnd() % 32u ? (uint16_t)(rnd() % 8u) : (uint16_t)(rnd() & 077777u);
    io->superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    hw->in_interrupt = (uint16_t)(rnd() % 32u ? 1u : rnd() % 3u);
    hw->channel10 = (uint16_t)(rnd() & 077777u);
    hw->channel32 = (uint16_t)(rnd() & 077777u);
}

static unsigned compare(const char *name, const agc_routine_table *readable,
                        const agc_routine_table *faithful)
{
    unsigned i, n, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL %s: %u entries, faithful %u\n", name, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z, fb;
        agc_routine f = faithful->entries[i].call, r = readable->entries[i].call;
        unsigned ran = 0, bad = 0;

        if (readable->entries[i].physical != key) {
            printf("FAIL %s: entry %u at key %u, faithful %u\n", name, i,
                   readable->entries[i].physical, key);
            ++failures;
            continue;
        }
        if (key >= 2048u && key < 4096u) { /* fixed-fixed */
            z = (uint16_t)(04000u + (key - 2048u));
            fb = (uint16_t)((rnd() % 040u) << 10);
        } else {
            z = (uint16_t)(02000u + key % 1024u);
            fb = (uint16_t)((key / 1024u) << 10);
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_hardware hw, fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, &hw, z, fb);
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hw;
            memset(&fp, 0, sizeof fp);
            memset(&rp, 0, sizeof rp);
            fp.hardware = &fhw;
            rp.hardware = &rhw;
            if (n % 64u == 63u) fp.hardware = rp.hardware = 0; /* no hardware record */
            sf = f(&fp, &fa, &fio);
            sr = r(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                                  memcmp(&rhw, &hw, sizeof rhw)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK &&
                     (memcmp(&fio, &rio, sizeof fio) || memcmp(&fhw, &rhw, sizeof fhw)))
                why = "different io or channels";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %s %05o case %u: %s\n", name, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %s %05o: the faithful routine never ran\n", name, z);
            ++bad;
        }
        failures += bad;
        printf("%s %s %05o: %u cases, %u run by the faithful routine, %u failures\n", MISSION, name,
               z, CASES, ran, bad);
    }
    return failures;
}

int main(void)
{
    unsigned t, failures = 0;

    for (t = 0; t < sizeof tables / sizeof tables[0]; ++t)
        failures += compare(tables[t].name, tables[t].readable, tables[t].faithful);
    printf("t4rupt-cases %s: %u failures\n", MISSION, failures);
    return failures != 0;
}
