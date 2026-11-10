/* SPDX-License-Identifier: MIT */
/* The readable KALMAN_FILTER against the faithful port's, case by case
 * (make kalman-filter-cases), the LM's only.
 *
 * Built with the readable tables (src/binding/kalman_filter_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them). RATELOOP runs on 65,536 random states:
 * random memory, mostly in bank 16 under EBANK 6 with the bank registers
 * agreeing; A, L and Q 16-bit registers; TJP, TJU and TJV mostly +0, -0,
 * a short firing (100 ms or less, 160 in TIME6's units, and the edge) or
 * a long one, of either sign; the jet counts NO.PJETS -V mostly 0 to 4;
 * and now and then a bank, a ZERO, a latch or a word the faithful routine
 * refuses. Where either port runs, the other must too, with the same
 * memory and io record, word for word; where the readable one refuses, it
 * must leave everything as it found it. The faithful routine is certified
 * against the original program (c64-port/tests/test_lm_kalman.c), as the
 * readable one is by make kalman-filter-cases' oracle step. */
#include <stdio.h>
#include <string.h>
#include "lm_kalman.h"
extern const agc_routine_table faithful_lm_kalman_basic;

/* EBANK 6 words, by their erasable addresses. */
#define NO_PJETS 01521u
#define TJP      01524u
#define E6(a)    (03000u + ((a) - 01400u))

#define CASES 65536u

static uint32_t seed = 3641u;
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
    return rnd() % 8u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

/* A jet time: +0, -0, short, at the 160 edge, or long, of either sign. */
static uint16_t jet_time(void)
{
    uint16_t v;

    switch (rnd() % 6u) {
    case 0: return 0u;
    case 1: return 077777u;
    case 2: v = (uint16_t)(rnd() % 161u); break;
    case 3: v = (uint16_t)(159u + rnd() % 3u); break;
    case 4: v = (uint16_t)(161u + rnd() % 600u); break;
    default: v = (uint16_t)(rnd() & 037777u); break;
    }
    return rnd() % 2u ? v : (uint16_t)(~v & 077777u);
}

static void state(apollo_overlay_state *s, p66_bias_io *io)
{
    unsigned i;
    uint16_t eb = rnd() % 32u ? 6u : (uint16_t)(rnd() % 8u);

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[2] = reg();
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = rnd() % 32u ? 034000u : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = 03641u;
    s->words[6] = (uint16_t)(s->words[4] | eb);
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    if (rnd() % 128u == 0u) s->words[010u + rnd() % (APOLLO_SHARED_WORDS - 010u)] = 0100000u;
    for (i = 0; i < 3u; ++i) {
        s->words[E6(TJP) + i] = jet_time();
        if (rnd() % 8u) s->words[E6(NO_PJETS) + i] = (uint16_t)(rnd() % 5u);
    }
    io->superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    if (rnd() % 64u == 0u) io->interrupt_enabled = 2u;
}

int main(void)
{
    unsigned n, ran = 0, failures = 0;
    agc_routine f = faithful_lm_kalman_basic.entries[0].call, r = lm_kalman_basic.entries[0].call;

    if (lm_kalman_basic.count != 1u || faithful_lm_kalman_basic.count != 1u ||
        lm_kalman_basic.entries[0].physical != faithful_lm_kalman_basic.entries[0].physical ||
        lm_kalman_interpretive.count != 0u) {
        printf("FAIL: the tables are not the faithful port's\n");
        return 1;
    }
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        p66_bias_io io, fio, rio;
        agc_hardware hw;
        agc_native_program p;
        enum exec_status sf, sr;
        const char *why = 0;
        int none = rnd() % 64u == 0u;

        state(&start, &io);
        fa = ra = start;
        fio = rio = io;
        memset(&p, 0, sizeof p);
        memset(&hw, 0, sizeof hw);
        p.hardware = n % 2u ? &hw : 0;
        sf = f(&p, &fa, none ? 0 : &fio);
        sr = r(&p, &ra, none ? 0 : &rio);
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
            why = "different io record";
        if (why && ++failures <= 3u) {
            unsigned w;
            printf("FAIL case %u: %s (status %d, faithful %d)\n", n, why, (int)sr, (int)sf);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    if (ran < CASES / 2u) {
        printf("FAIL: the faithful routine ran in %u of %u states\n", ran, CASES);
        ++failures;
    }
    printf("kalman-filter-cases: %u states, %u faithful runs, %u failures\n", CASES, ran, failures);
    return failures != 0u;
}
