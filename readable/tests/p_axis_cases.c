/* SPDX-License-Identifier: MIT */
/* The readable P-AXIS_RCS_AUTOPILOT against the faithful port's, key by key
 * (make p-axis-cases), the LM's only.
 *
 * Built with the readable tables (src/binding/p_axis_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them), and the ropes loaded (tests/with_ropes.c: the
 * readable routines read the file's tables from the rope). First the keys
 * of the three tables, which must be the faithful tables'. Then the runs:
 * each key from 4,096 states, each the port's case image
 * (c64-port/tests/lm_p_axis_cases.c: random memory with +0 and -0 frequent,
 * FB bank 16, EBANK 6, channels 12, 13 and 31 random, a random interrupt
 * enable) entered at the key, most stirred further: the bits of RCSFLAGS,
 * DAPBOOLS and channel 31 the file tests, A (now and then an overflow), L
 * (never overflowed: the readable machine corrects it after each
 * instruction, the faithful routines leave it), Q (a return in fixed
 * memory, now and then in erasable memory or past it), the gimbal timers,
 * OLDPMIN, TJP and the trap sums small, NUMBERT in A for PJETSLEC -1 (4, 5
 * or 6: the jet types TYPEP has), ROTINDEX +0 or 1 for PJETSLEC +25 (now
 * and then not), EBANK and FB now and then another bank, and the hardware
 * record now and then missing. Each runs once on each side, as the
 * program's trampoline would call the key's routine. Where either port
 * runs, the other must too, with the same memory, io record and channels,
 * word for word; where the readable one refuses, it must leave everything
 * as it found it; and the faithful routine must run in at least 1 of 16
 * of each key's states. */
#include <stdio.h>
#include <string.h>
#include "lm_p_axis_cases.h"

extern const agc_routine_table faithful_lm_p_axis_routines;
extern const agc_routine_table faithful_lm_p_axis_hot_routines;
extern const agc_routine_table faithful_lm_p_axis_tg_hot_routines;

#define CASES 4096u

/* Words the routines test, by their erasable addresses: unswitched
 * RCSFLAGS, DAPBOOLS; in EBANK 6 the rest. */
#define RCSFLAGS 01273u
#define DAPBOOLS 0111u
#define TRAPEDP  01430u
#define OLDPMIN  01460u
#define TJP      01524u
#define QGIMTIMR 01632u
#define RGIMTIMR 01634u
#define ROTINDEX 01744u
#define E6(a)    (03000u + ((a) - 01400u))

static uint32_t seed = 2213u;
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

/* A small number, either sign, now and then +0 or -0. */
static uint16_t small(void)
{
    uint16_t v = (uint16_t)pick(8u);
    return pick(2u) ? v : (uint16_t)(~v & 077777u);
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

static void state(apollo_overlay_state *s, lm_flight_io *io, unsigned entry, uint16_t key,
                  unsigned n)
{
    uint16_t *w = s->words;
    unsigned i;

    memset(io, 0, sizeof *io);
    lm_pa_input(s, io, (uint16_t)((n * LM_PA_ENTRIES + entry) % LM_PA_CASES));
    if (key == LM_PA_PJETSLEC1) w[0] = (uint16_t)(4u + pick(3u));
    if (key == LM_PA_WRITEPRET) w[E6(ROTINDEX)] = (uint16_t)pick(2u);
    if (n % 4u == 0u) return;
    for (i = pick(4u); i > 0u; --i) flip(&w[RCSFLAGS], 07421u);
    for (i = pick(4u); i > 0u; --i) flip(&w[DAPBOOLS], 054620u);
    for (i = pick(4u); i > 0u; --i) flip(&io->hw.channel31, 057414u);
    if (pick(4u) == 0u) w[E6(OLDPMIN)] = small();
    if (pick(4u) == 0u) w[E6(TJP)] = small();
    if (pick(4u) == 0u) w[E6(QGIMTIMR)] = small();
    if (pick(4u) == 0u) w[E6(RGIMTIMR)] = small();
    if (pick(4u) == 0u) w[E6(TRAPEDP + pick(3u))] = small();
    if (pick(2u) && key != LM_PA_PJETSLEC1) w[0] = register_word(any_word());
    if (pick(2u)) w[1] = sign_extended(any_word());
    if (pick(4u) == 0u) w[2] = return_word();
    if (key == LM_PA_WRITEPRET && pick(16u) == 0u) w[E6(ROTINDEX)] = pick(2u) ? 2u : 077777u;
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
        uint16_t z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0, entry;

        for (entry = 0; entry < LM_PA_ENTRIES; ++entry)
            if (lm_pa_entry_z[entry] == z) break;
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            lm_flight_io io, fio, rio;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;
            int none = pick(64u) == 0u;

            state(&start, &io, entry < LM_PA_ENTRIES ? entry : n % LM_PA_ENTRIES, z, n);
            start.words[5] = z;
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
                printf("FAIL 16,%04o case %u: %s (status %d, faithful %d)\n", z, n, why, (int)sr,
                       (int)sf);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL 16,%04o: the faithful routine ran in %u of %u states\n", z, ran, CASES);
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

int port_main(void)
{
    unsigned failures = 0, runs = 0;

    failures += same_keys(&lm_p_axis_routines, &faithful_lm_p_axis_routines);
    failures += same_keys(&lm_p_axis_hot_routines, &faithful_lm_p_axis_hot_routines);
    failures += same_keys(&lm_p_axis_tg_hot_routines, &faithful_lm_p_axis_tg_hot_routines);
    if (failures) return 1;
    failures = keys(&lm_p_axis_routines, &faithful_lm_p_axis_routines, &runs);
    printf("p-axis-cases: %u keys from %u states each, %u faithful runs, %u failures\n",
           lm_p_axis_routines.count, CASES, runs, failures);
    return failures != 0u;
}
