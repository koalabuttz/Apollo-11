/* SPDX-License-Identifier: MIT */
/* The readable SMALLMP and DPADD against the faithful port's, case by case
 * (make cm-mysubs-cases).
 *
 * Built with the readable table (src/binding/cm_mysubs_routines.c) and the
 * faithful port's under the same name with faithful_ in front (the
 * Makefile renames it). Both entries run on every value of A's 16 bits
 * (overflows too), four times each with random L (often an overflow too),
 * Q, KMPAC and memory: KMPAC is now and then the word that brings the sum
 * to exactly an overflow (DPADD's upper word then +0 or -0), and its lower
 * word now and then the one that brings that word to +0 or -0. Banks are
 * mostly the routines' (FB bank 21, EBANK E6), now and then any, and now
 * and then Q is outside fixed memory (often erasable) or ZERO is not +0. Where the faithful
 * routine runs, the readable one must too, with the same erasable memory
 * and registers, word for word; where the readable one refuses, it must
 * leave the state as it found it. The faithful routines are certified
 * against the original program (c64-port/tests/test_cm_mysubs.c). */
#include <stdio.h>
#include <string.h>
#include "cm_mysubs_routines.h"
#include "../src/agc/names.h"

extern const agc_routine_table faithful_cm_mysubs_routines;

#define ROUNDS     4u
#define KMPAC_AT   ((unsigned)K_KMPAC + 01400u) /* E6: 03000 + (01510 - 01400) */
#define KMPTEMP_AT ((unsigned)K_KMPTEMP + 01400u)

static uint32_t seed = 999u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* A word of a number: often an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 037777u,
                                     040000u, 020000u, 057777u, 000002u, 077775u};
    return (rnd() >> 6) % 4u ? (uint16_t)(rnd() & 077777u)
                             : edges[(rnd() >> 6) % (sizeof edges / sizeof edges[0])];
}

/* A 16-bit register: mostly a number sign-extended, now and then an
 * overflow. */
static uint16_t register_word(void)
{
    uint16_t v = value_word();

    v = (uint16_t)(v & 040000u ? v | 0100000u : v);
    return (rnd() >> 6) % 4u ? v : (uint16_t)(v ^ 0100000u);
}

/* The 15-bit word that, added to v (ones' complement, 15 bits), makes the
 * sum exactly +-040000, an overflow with nothing left: +0 or -0. */
static uint16_t to_overflow(uint16_t v)
{
    v &= 077777u;
    if (v == 0u || v == 077777u) return value_word();
    return v < 040000u ? (uint16_t)(040000u - v) : (uint16_t)(077777u - (040000u - (077777u - v)));
}

/* The 15-bit word that, added to v, makes +0 or -0: its negative. */
static uint16_t to_zero(uint16_t v)
{
    return (uint16_t)(~v & 077777u);
}

static void state(apollo_overlay_state *s, uint16_t z, uint16_t a)
{
    unsigned i;

    memset(s, 0, sizeof *s);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = a;
    s->words[1] = register_word();
    s->words[2] = (rnd() >> 6) % 16u  ? (uint16_t)(02000u + rnd() % 06000u)
                  : (rnd() >> 6) % 2u ? (uint16_t)(rnd() % 02000u)
                                      : (uint16_t)(rnd() | (rnd() << 1));
    s->words[3] = (rnd() >> 6) % 8u ? 03000u : (uint16_t)(((rnd() >> 6) % 8u) << 8);
    s->words[4] = (rnd() >> 6) % 8u ? 042000u : (uint16_t)(((rnd() >> 6) % 040u) << 10);
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
    if ((rnd() >> 6) % 64u == 0u) s->words[6] = (uint16_t)(rnd() & 077777u);
    s->words[5] = z;
    if ((rnd() >> 6) % 64u == 0u) s->words[7] = 1;
    s->words[KMPAC_AT] = value_word();
    s->words[KMPAC_AT + 1u] = value_word();
    s->words[KMPTEMP_AT] = value_word();
    if ((rnd() >> 6) % 4u == 0u) s->words[KMPAC_AT] = to_overflow(a);
    if ((rnd() >> 6) % 4u == 0u) s->words[KMPAC_AT + 1u] = to_zero(s->words[1]);
}

int main(void)
{
    agc_native_program program;
    unsigned i, checks = 0, failures = 0;

    memset(&program, 0, sizeof program);
    if (cm_mysubs_routines.count != faithful_cm_mysubs_routines.count) {
        printf("FAIL: %u entries, faithful %u\n", cm_mysubs_routines.count,
               faithful_cm_mysubs_routines.count);
        return 1;
    }
    for (i = 0; i < faithful_cm_mysubs_routines.count; ++i) {
        uint16_t key = faithful_cm_mysubs_routines.entries[i].physical,
                 z = (uint16_t)(02000u + key % 1024u);
        agc_routine f = faithful_cm_mysubs_routines.entries[i].call,
                    r = cm_mysubs_routines.entries[i].call;
        unsigned a, round, ran = 0, bad = 0;
        if (cm_mysubs_routines.entries[i].physical != key) {
            printf("FAIL: entry %u at key %u, faithful %u\n", i,
                   cm_mysubs_routines.entries[i].physical, key);
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
                else if (sf != EXEC_OK && sr == EXEC_OK)
                    why = "readable ran, faithful refused";
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
        printf("CM %s %05o: %u cases, %u run by the faithful routine, %u failures\n",
               i ? "DPADD" : "SMALLMP", z, 0200000u * ROUNDS, ran, bad);
        if (ran < 0200000u * ROUNDS / 2u) {
            printf("FAIL: the faithful routine ran in too few cases\n");
            ++failures;
        }
    }
    printf("cm-mysubs-cases: %u checks, %u failures\n", checks, failures);
    return failures != 0;
}
