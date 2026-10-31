/* SPDX-License-Identifier: MIT */
/* The readable SPSIN and SPCOS against the faithful port's, case by case
 * (make sp-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * sp_routines.c) and the faithful port's under the same name with
 * faithful_ in front (the Makefile renames it). Both entries run on every
 * value of A's 16 bits (overflows too), four times each with random L, Q,
 * memory and banks: where the faithful routine runs, the readable one must
 * too, with the same erasable memory and registers, word for word; where
 * the readable one refuses, it must leave the state as it found it. The
 * faithful routines are certified against the original program
 * (c64-port/tests/test_sp_routines.c), as the readable ones are by
 * make sp-cases' first step. */
#include <stdio.h>
#include <string.h>
#include "sp_routines.h"
#if LUMINARY099
#define MISSION  "LM"
#define TABLE    lm_sp_routines
#define FAITHFUL faithful_lm_sp_routines
#else
#define MISSION  "CM"
#define TABLE    cm_sp_routines
#define FAITHFUL faithful_cm_sp_routines
#endif

extern const agc_routine_table FAITHFUL;

#define ROUNDS 4u

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static void state(apollo_overlay_state *s, uint16_t z, uint16_t a)
{
    unsigned i;

    memset(s, 0, sizeof *s);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = a;
    s->words[1] = (uint16_t)(rnd() | (rnd() << 1));
    s->words[2] =
        rnd() % 16u ? (uint16_t)(02000u + rnd() % 06000u) : (uint16_t)(rnd() | (rnd() << 1));
    s->words[3] = (uint16_t)((rnd() % 8u) << 8);
    s->words[4] = (uint16_t)((rnd() % 040u) << 10);
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
    s->words[5] = z;
    if (rnd() % 64u == 0u) s->words[7] = 1;
}

int main(void)
{
    agc_native_program program;
    unsigned i, checks = 0, failures = 0;

    memset(&program, 0, sizeof program);
    if (TABLE.count != FAITHFUL.count) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < FAITHFUL.count; ++i) {
        uint16_t key = FAITHFUL.entries[i].physical, z = (uint16_t)(04000u + key - 2048u);
        agc_routine f = FAITHFUL.entries[i].call, r = TABLE.entries[i].call;
        unsigned a, round, ran = 0, bad = 0;
        if (TABLE.entries[i].physical != key) {
            printf("FAIL: entry %u at key %u, faithful %u\n", i, TABLE.entries[i].physical, key);
            ++failures;
            continue;
        }
        for (a = 0; a < 0200000u; ++a)
            for (round = 0; round < ROUNDS; ++round) {
                apollo_overlay_state fa, ra, start;
                enum exec_status sf, sr;
                const char *why = 0;
                state(&start, z, (uint16_t)a);
                fa = ra = start;
                sf = f(&program, &fa, 0);
                sr = r(&program, &ra, 0);
                ++checks;
                ran += sf == EXEC_OK;
                if (sr != EXEC_OK && memcmp(&ra, &start, sizeof ra))
                    why = "readable refusal changed the state";
                else if (sf == EXEC_OK && sr != EXEC_OK)
                    why = "readable refused, faithful ran";
                else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                    why = "different";
                if (why && ++bad <= 3) {
                    unsigned w;
                    printf("FAIL %05o A %06o: %s\n", z, a, why);
                    for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                        if (fa.words[w] != ra.words[w])
                            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w,
                                   ra.words[w], fa.words[w], start.words[w]);
                }
            }
        failures += bad;
        printf("%s %s %05o: %u cases, %u run by the faithful routine, %u failures\n", MISSION,
               i ? "SPSIN" : "SPCOS", z, 0200000u * ROUNDS, ran, bad);
    }
    printf("sp-cases %s: %u checks, %u failures\n", MISSION, checks, failures);
    return failures != 0;
}
