/* SPDX-License-Identifier: MIT */
/* The readable TJET_LAW against the faithful port's (make
 * tjet-law-cases), the LM's only.
 *
 * Built with the readable table (src/binding/tjet_law_routines.c) and the
 * faithful port's under the same name with faithful_ in front (the
 * Makefile renames it), and the ropes loaded (tests/with_ropes.c: the
 * readable routine reads AXISDIFF and ZON1,2,3's word from the rope).
 * First the key, which must be the faithful table's. Then 65,536 runs of
 * it, each from the port's case image (c64-port/tests/lm_tjet_law_cases.c:
 * random memory, FB bank 17, EBANK 6, AXISCTR -1, +0 or +1, a return in
 * Q, and cohorts with E, EDOT and the 1/ACCS parameters in the fine law's
 * ranges, at the rough laws' edges or full range), most stirred further:
 * E and EDOT +0, -0, small or any word; SENSETYP and FLAT +0, -0 or of
 * either sign; TJP, TJU, TJV and ACCSWU, -V of either sign; and now and
 * then an AXISCTR, a return in Q or a bank the faithful routine refuses,
 * and the io record missing. Each runs once on each side, as the
 * program's trampoline calls TJETLAW. Where either port runs, the other
 * must too, with the same memory, word for word; where the readable one
 * refuses, it must leave the state as it found it. */
#include <stdio.h>
#include <string.h>
#include "lm_tjet_law_cases.h"
#include "p66_bias.h"

extern const agc_routine_table faithful_lm_tjet_law_routines;

#define RUNS 65536u

/* EBANK 6 words, by their erasable addresses. */
#define EDOT     01427u
#define SENSETYP 01500u
#define TJP      01524u
#define ACCSWU   01547u
#define FLAT     01555u
#define E        01752u
#define E6(a)    (03000u + ((a) - 01400u))

static uint32_t seed = 3252u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
}

/* +0, -0, a small number of either sign, or any word. */
static uint16_t stirred(void)
{
    uint16_t v = (uint16_t)pick(64u);

    switch (pick(5u)) {
    case 0: return 0u;
    case 1: return 077777u;
    case 2: return v;
    case 3: return (uint16_t)(~v & 077777u);
    default: return (uint16_t)(rnd() & 077777u);
    }
}

static void state(apollo_overlay_state *s, unsigned n)
{
    uint16_t *w = s->words;
    lm_tj_case c;
    unsigned i;

    lm_tj_input(s, &c, (uint16_t)(n % LM_TJ_CASES));
    if (n % 4u == 0u) return;
    if (pick(4u) == 0u) w[E6(E)] = stirred();
    if (pick(4u) == 0u) w[E6(EDOT)] = stirred();
    if (pick(4u) == 0u) w[E6(SENSETYP)] = stirred();
    if (pick(4u) == 0u) w[E6(FLAT)] = stirred();
    for (i = 0; i < 3u; ++i)
        if (pick(4u) == 0u) w[E6(TJP) + i] = stirred();
    for (i = 0; i < 2u; ++i)
        if (pick(4u) == 0u) w[E6(ACCSWU) + i] = stirred();
    if (pick(32u) == 0u) w[E6(LM_TJ_AXISCTR)] = stirred();
    if (pick(32u) == 0u)
        w[2] = pick(2u) ? (uint16_t)pick(02000u) : (uint16_t)(010000u + pick(010000u));
    if (pick(64u) == 0u) {
        w[3] = (uint16_t)(pick(8u) << 8);
        w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
    }
    if (pick(64u) == 0u) {
        w[4] = (uint16_t)(rnd() & 076000u);
        w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
    }
}

static unsigned same_key(void)
{
    if (lm_tjet_law_routines.count != 1u || faithful_lm_tjet_law_routines.count != 1u ||
        lm_tjet_law_routines.entries[0].physical !=
            faithful_lm_tjet_law_routines.entries[0].physical) {
        printf("FAIL: the key is not the faithful table's\n");
        return 1u;
    }
    return 0u;
}

int port_main(void)
{
    unsigned n, ran = 0, failures = same_key();
    agc_native_program program;

    if (failures) return 1;
    memset(&program, 0, sizeof program);
    for (n = 0; n < RUNS; ++n) {
        apollo_overlay_state start, fa, ra;
        p66_bias_io io, fio, rio;
        void *fd = 0, *rd = 0;
        enum exec_status sf, sr;
        const char *why = 0;

        state(&start, n);
        fa = ra = start;
        memset(&io, 0, sizeof io);
        fio = rio = io;
        if (pick(4u)) {
            fd = &fio;
            rd = &rio;
        }
        sf = faithful_lm_tjet_law_routines.entries[0].call(&program, &fa, fd);
        sr = lm_tjet_law_routines.entries[0].call(&program, &ra, rd);
        ran += sf == EXEC_OK;
        if (sr != EXEC_OK && memcmp(&ra, &start, sizeof ra))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK)
            why = "readable refused, faithful ran";
        else if (sf != EXEC_OK && sr == EXEC_OK)
            why = "readable ran, faithful refused";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (memcmp(&fio, &rio, sizeof fio))
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
    printf("tjet-law-cases: %u runs, %u faithful runs, %u failures\n", RUNS, ran, failures);
    return failures != 0u;
}
