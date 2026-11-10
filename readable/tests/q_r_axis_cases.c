/* SPDX-License-Identifier: MIT */
/* The readable Q_R-AXIS_RCS_AUTOPILOT against the faithful port's, key by
 * key (make q-r-axis-cases), the LM's only.
 *
 * Built with the readable tables (src/binding/q_r_axis_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them), and the ropes loaded (tests/with_ropes.c: the
 * readable routines read the file's tables from the rope). First the keys
 * of the three tables, which must be the faithful tables'. Then the runs:
 * each key from 4,096 states, each the port's case image
 * (c64-port/tests/lm_q_r_axis_cases.c: random memory with +0 and -0
 * frequent, FB bank 17, EBANK 6, channels 4, 13 and 31 random, channel 5
 * eight jet bits, a random interrupt enable; SKIPU, SKIPV, RETJADR,
 * AXISCTR, NUMBERT and SENSETYP mostly as the file stores them) entered
 * at the key, most stirred further: the bits of DAPBOOLS, RCSFLAGS,
 * FLAGWRD5, FLGWRD10, CH31TEMP and channel 31 the file tests, the failed
 * jets of CH5MASK and channel 5, the counters and times it tests small
 * (OLDQRMIN, TCQR, INGTS, ALLOWGTS, COTROLER, the hand controller, TJU,
 * TJV, ABSTJ, the rate differences, the T6 list), A (now and then an
 * overflow), L (never overflowed: the readable machine corrects it after
 * each instruction, the faithful routines leave it), Q (a return in fixed
 * memory, now and then in erasable memory or past it), EBANK and FB now
 * and then another bank, and the hardware record now and then missing.
 * The keys the file's own runs hand Z on to are entered, as those runs
 * enter them, with the axis in A at MINRTN, BACKHAND and TJLAW +3, and
 * SKIPU and SKIPV 0 or 4; at DOSKIP and DOSKIP +1 with AXISCTR 0 or 1,
 * ROTINDEX 0-3, NUMBERT 0-4 and SENSETYP 0-5 or negative; at TRYGTS with
 * ALLOWGTS not -0 and channel 5 not negative: the faithful routines take
 * any state there, and read their jet tables past their ends with others.
 * Now and then such a key is entered otherwise: the readable routine must
 * then refuse, leaving everything as it found it, or run. Each runs once
 * on each side, as the program's trampoline would call the key's routine.
 * Where either port runs, the other must too, with the same memory, io
 * record and channels, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * in at least 1 of 16 of each key's states. */
#include <stdio.h>
#include <string.h>
#include "lm_q_r_axis_cases.h"

extern const agc_routine_table faithful_lm_q_r_axis_routines;
extern const agc_routine_table faithful_lm_q_r_axis_hot_routines;
extern const agc_routine_table faithful_lm_q_r_axis_tg_hot_routines;

#define CASES 4096u

/* Words the routines test, by their erasable addresses: unswitched
 * FLAGWRD5, FLGWRD10, DAPBOOLS, CH5MASK, RCSFLAGS; in EBANK 6 the rest. */
#define FLAGWRD5 0101u
#define FLGWRD10 0106u
#define DAPBOOLS 0111u
#define CH5MASK  01262u
#define RCSFLAGS 01273u
#define OMEGAU   01426u
#define EDOTQ    01436u
#define CH31TEMP 01443u
#define OLDQRMIN 01461u
#define SAVEHAND 01462u
#define T6NEXT   01466u
#define T6FURTHA 01470u
#define RETJADR  01477u
#define SENSETYP 01500u
#define ALLOWGTS 01502u
#define AXISCTR  01505u
#define TJU      01525u
#define SKIPU    01535u
#define COTROLER 01631u
#define INGTS    01633u
#define TCQR     01457u
#define ABSTJ    01737u
#define NUMBERT  01743u
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

/* The keys the file's own runs hand Z on to that read state those runs
 * set: whether the state is one they would hand on. */
static int in_domain(const uint16_t *w, const lm_flight_io *io, uint16_t key)
{
    uint16_t sensetyp = w[E6(SENSETYP)];

    if ((key == LM_QR_BACKHAND || key == LM_QR_STILLRCS || key == LM_QR_TJLAW3) &&
        ((w[E6(SKIPU)] != 0u && w[E6(SKIPU)] != 4u) ||
         (w[E6(SKIPU) + 1u] != 0u && w[E6(SKIPU) + 1u] != 4u)))
        return 0;
    switch (key) {
    case LM_QR_MINRTN:
    case LM_QR_BACKHAND:
    case LM_QR_TJLAW3: return w[0] <= 1u;
    case LM_QR_DOSKIP:
    case LM_QR_DOSKIP1:
        return w[E6(AXISCTR)] <= 1u && w[E6(ROTINDEX)] <= 3u && w[E6(NUMBERT)] <= 4u &&
               (sensetyp <= 5u || sensetyp >= 040000u);
    case LM_QR_TRYGTS: return w[E6(ALLOWGTS)] != 077777u && !(io->hw.channel5 & 040000u);
    default: return 1;
    }
}

static int is_part(uint16_t key)
{
    unsigned i;

    for (i = 0; i < LM_QR_ENTRIES; ++i)
        if (lm_qr_entry_z[i] == key) return 0;
    return 1;
}

static void state(apollo_overlay_state *s, lm_flight_io *io, unsigned entry, uint16_t key,
                  unsigned n)
{
    uint16_t *w = s->words;
    unsigned i;

    memset(io, 0, sizeof *io);
    lm_qr_input(s, io, (uint16_t)((n * LM_QR_ENTRIES + entry) % LM_QR_CASES));
    if (n % 4u != 0u) {
        for (i = pick(4u); i > 0u; --i) flip(&w[DAPBOOLS], 077240u);
        for (i = pick(3u); i > 0u; --i) flip(&w[RCSFLAGS], 02400u);
        if (pick(4u) == 0u) flip(&w[FLAGWRD5], 010000u);
        if (pick(4u) == 0u) flip(&w[FLGWRD10], 010000u);
        if (pick(4u) == 0u) flip(&w[E6(CH31TEMP)], 040000u);
        for (i = pick(4u); i > 0u; --i) flip(&io->hw.channel31, 010363u);
        if (pick(2u)) w[CH5MASK] = pick(2u) ? 0u : (uint16_t)(1u << pick(8u));
        if (pick(2u)) io->hw.channel5 = pick(2u) ? 0u : (uint16_t)(1u << pick(8u));
        if (pick(4u) == 0u) w[E6(OLDQRMIN)] = small();
        if (pick(4u) == 0u) w[E6(TCQR)] = small();
        if (pick(4u) == 0u) w[E6(INGTS)] = small();
        if (pick(4u) == 0u) w[E6(ALLOWGTS)] = (uint16_t)pick(2u);
        if (pick(4u) == 0u) w[E6(COTROLER)] = small();
        if (pick(4u) == 0u) w[E6(SAVEHAND) + pick(2u)] = small();
        if (pick(4u) == 0u) w[E6(TJU) + pick(2u)] = small();
        if (pick(4u) == 0u) w[E6(ABSTJ)] = small();
        if (pick(4u) == 0u) w[E6(OMEGAU) + pick(2u)] = small();
        if (pick(4u) == 0u) w[E6(EDOTQ) + pick(2u)] = small();
        if (pick(4u) == 0u) w[E6(T6NEXT) + pick(2u)] = small();
        if (pick(4u) == 0u) w[E6(T6FURTHA)] = small();
        if (pick(2u)) w[0] = register_word(any_word());
        if (pick(2u)) w[1] = sign_extended(any_word());
        if (pick(4u) == 0u) w[2] = return_word();
        if (pick(32u) == 0u) {
            w[3] = (uint16_t)(pick(8u) << 8);
            w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
        }
        if (pick(32u) == 0u) {
            w[4] = (uint16_t)(rnd() & 076000u);
            w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
        }
    }
    if (!is_part(key) || pick(16u) == 0u) return;
    /* As the file's own runs enter a key they hand Z on to. */
    w[E6(SKIPU)] = pick(2u) ? 4u : 0u;
    w[E6(SKIPU) + 1u] = pick(2u) ? 4u : 0u;
    if (key == LM_QR_MINRTN || key == LM_QR_BACKHAND || key == LM_QR_TJLAW3)
        w[0] = (uint16_t)pick(2u);
    if (key == LM_QR_DOSKIP || key == LM_QR_DOSKIP1) {
        w[E6(AXISCTR)] = (uint16_t)pick(2u);
        w[E6(ROTINDEX)] = (uint16_t)pick(4u);
        w[E6(NUMBERT)] = (uint16_t)pick(5u);
        if (w[E6(SENSETYP)] > 5u && w[E6(SENSETYP)] < 040000u) w[E6(SENSETYP)] = (uint16_t)pick(3u);
    }
    if (key == LM_QR_TRYGTS) {
        if (w[E6(ALLOWGTS)] == 077777u) w[E6(ALLOWGTS)] = 0u;
        io->hw.channel5 &= 0377u;
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

        for (entry = 0; entry < LM_QR_ENTRIES; ++entry)
            if (lm_qr_entry_z[entry] == z) break;
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            lm_flight_io io, fio, rio;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;
            int none = pick(64u) == 0u;

            state(&start, &io, entry < LM_QR_ENTRIES ? entry : n % LM_QR_ENTRIES, z, n);
            start.words[5] = z;
            fa = ra = start;
            fio = rio = io;
            memset(&fp, 0, sizeof fp);
            memset(&rp, 0, sizeof rp);
            fp.hardware = none ? 0 : &fio.hw;
            rp.hardware = none ? 0 : &rio.hw;
            if (is_part(z) && !in_domain(start.words, &io, z)) {
                sr = readable->entries[i].call(&rp, &ra, &rio);
                if (sr != EXEC_OK &&
                    (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio))) {
                    printf("FAIL 17,%04o case %u: readable refusal changed the state\n", z, n);
                    ++bad;
                }
                continue;
            }
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
                printf("FAIL 17,%04o case %u: %s (status %d, faithful %d)\n", z, n, why, (int)sr,
                       (int)sf);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL 17,%04o: the faithful routine ran in %u of %u states\n", z, ran, CASES);
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

    failures += same_keys(&lm_q_r_axis_routines, &faithful_lm_q_r_axis_routines);
    failures += same_keys(&lm_q_r_axis_hot_routines, &faithful_lm_q_r_axis_hot_routines);
    failures += same_keys(&lm_q_r_axis_tg_hot_routines, &faithful_lm_q_r_axis_tg_hot_routines);
    if (failures) return 1;
    failures = keys(&lm_q_r_axis_routines, &faithful_lm_q_r_axis_routines, &runs);
    printf("q-r-axis-cases: %u keys from %u states each, %u faithful runs, %u failures\n",
           lm_q_r_axis_routines.count, CASES, runs, failures);
    return failures != 0u;
}
