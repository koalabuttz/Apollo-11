/* SPDX-License-Identifier: MIT */
/* The readable DAPIDLER_PROGRAM against the faithful port's, key by key
 * (make dapidler-cases), the LM's only.
 *
 * Built with the readable tables (src/binding/dapidler_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them). First the keys, which must be the faithful
 * tables'. Then the runs: each key from 4,096 states, each the port's case
 * image (c64-port/tests/lm_dapidler_cases.c: any memory, FB bank 16, or
 * bank 20 for ALTDSPLY, EBANK 6, channels 5, 6, 12, 30 and 31 any, a
 * caller's return in Q) entered at the key, most stirred further: the
 * bits RCSFLAGS, IMODES33, DAPBOOLS and FLAGWRD0 are tested by, those of
 * channels 12, 30 and 31, A (now and then an overflow), L (never
 * overflowed: the readable machine corrects it after each instruction,
 * the faithful routines leave it), Q (a return in fixed memory, now and
 * then in erasable memory or past it), the attitude errors AK (now and
 * then small, for the DAC loop's +0 and -0), the gimbal-to-pilot matrix
 * (now and then large, for OVERSUB2's limits), EBANK and FB now and then
 * another bank, and the hardware record now and then missing. Each runs
 * once on each side, as the program's trampoline would call the key's
 * routine. Where either port runs, the other must too, with the same
 * memory, io record and channels, word for word; where the readable one
 * refuses, it must leave everything as it found it; and the faithful
 * routine must run in at least 1 of 16 of each key's states. */
#include <stdio.h>
#include <string.h>
#include "lm_dapidler_cases.h"

extern const agc_routine_table faithful_lm_dapidler_routines;
extern const agc_routine_table faithful_lm_dapidler_hot_routines;

#define CASES 4096u

/* Words the routines test, by their erasable addresses: RCSFLAGS and
 * IMODES33 unswitched, FLAGWRD0, DAPBOOLS; in EBANK 6, AK and the matrix
 * M11 .. M32. */
#define RCSFLAGS 01273u
#define IMODES33 01303u
#define FLAGWRD0 074u
#define DAPBOOLS 0111u
#define AK       01761u
#define M11      01414u
#define E6(a)    (03000u + ((a) - 01400u))

static uint32_t seed = 1410u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
}

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

static uint16_t sign_extended(uint16_t v)
{
    return (uint16_t)(v & 040000u ? v | 0100000u : v);
}

/* A: mostly a number sign-extended, now and then an overflow. */
static uint16_t register_word(uint16_t v)
{
    v = sign_extended(v);
    return pick(8u) ? v : (uint16_t)(v ^ 0100000u);
}

/* A return word: mostly in fixed memory, now and then not. */
static uint16_t return_word(void)
{
    unsigned k = pick(8u);

    return k < 5u    ? (uint16_t)(02000u + pick(06000u))
           : k == 5u ? (uint16_t)pick(02000u)
           : k == 6u ? (uint16_t)(010000u + pick(070000u))
                     : (pick(2u) ? 01777u : 010000u);
}

/* A word with one of bits flipped. */
static void flip(uint16_t *w, uint16_t bits)
{
    uint16_t bit;

    do bit = (uint16_t)(1u << pick(15u));
    while (!(bits & bit));
    *w ^= bit;
}

static void state(apollo_overlay_state *s, lm_flight_io *io, unsigned entry, unsigned n)
{
    lm_di_case c;
    uint16_t *w = s->words;
    unsigned i;

    memset(io, 0, sizeof *io);
    lm_di_input(s, io, &c, (uint16_t)((n * LM_DI_ENTRIES + entry) % LM_DI_CASES));
    if (n % 4u == 0u) return;
    if (pick(2u)) flip(&w[RCSFLAGS], 010016u | 04000u);
    if (pick(4u) == 0u) flip(&w[IMODES33], 040u);
    if (pick(4u) == 0u) flip(&w[DAPBOOLS], 4u);
    if (pick(4u) == 0u) flip(&w[FLAGWRD0], 010u);
    if (pick(4u) == 0u) flip(&io->hw.channel12, 040u);
    if (pick(4u) == 0u) flip(&io->hw.channel30, 01000u);
    if (pick(4u) == 0u) io->hw.channel31 |= 030000u;
    if (pick(2u)) w[0] = register_word(any_word());
    if (pick(2u)) w[1] = sign_extended(any_word());
    if (pick(4u) == 0u) w[2] = return_word();
    if (pick(4u) == 0u) {
        uint16_t a = (uint16_t)pick(16u);
        w[E6(AK + pick(3u))] = pick(2u) ? a : (uint16_t)(~a & 077777u);
    }
    if (pick(4u) == 0u)
        for (i = 0; i < 5u; ++i)
            w[E6(M11 + i)] = (uint16_t)(pick(2u) ? 037777u - pick(64u) : 040000u + pick(64u));
    if (pick(32u) == 0u) {
        w[3] = (uint16_t)(pick(8u) << 8);
        w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
    }
    if (pick(32u) == 0u) {
        w[4] = (uint16_t)(rnd() & 076000u);
        w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
    }
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        unsigned ran = 0, bad = 0, entry;

        for (entry = 0; entry < LM_DI_ENTRIES; ++entry) {
            uint16_t z = lm_di_entry_z[entry];
            if ((uint16_t)((entry == 8u ? 020u : 016u) * 1024u + (z - 02000u)) == key) break;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            lm_flight_io io, fio, rio;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;
            int none = pick(64u) == 0u;

            state(&start, &io, entry, n);
            start.words[5] = (uint16_t)(02000u + key % 1024u);
            fa = ra = start;
            fio = rio = io;
            memset(&fp, 0, sizeof fp);
            memset(&rp, 0, sizeof rp);
            fp.hardware = none ? 0 : &fio.hw;
            rp.hardware = none ? 0 : &rio.hw;
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
                why = "different io record or channels";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %02o,%04o case %u: %s (status %d, faithful %d)\n", key / 1024u,
                       02000u + key % 1024u, n, why, (int)sr, (int)sf);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states\n", key / 1024u,
                   02000u + key % 1024u, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

static unsigned same_keys(const agc_routine_table *r, const agc_routine_table *f)
{
    unsigned i, failures = 0;

    if (r->count != f->count) {
        printf("FAIL: %u keys, faithful %u\n", r->count, f->count);
        return 1;
    }
    for (i = 0; i < r->count; ++i)
        if (r->entries[i].physical != f->entries[i].physical) {
            printf("FAIL: key %u is %u, faithful %u\n", i, r->entries[i].physical,
                   f->entries[i].physical);
            ++failures;
        }
    return failures;
}

int main(void)
{
    unsigned failures = 0, runs = 0;

    failures += same_keys(&lm_dapidler_routines, &faithful_lm_dapidler_routines);
    failures += same_keys(&lm_dapidler_hot_routines, &faithful_lm_dapidler_hot_routines);
    if (failures) return 1;
    failures = keys(&lm_dapidler_routines, &faithful_lm_dapidler_routines, &runs);
    printf("dapidler-cases: %u keys from %u states each, %u faithful runs, %u failures\n",
           lm_dapidler_routines.count, CASES, runs, failures);
    return failures != 0u;
}
