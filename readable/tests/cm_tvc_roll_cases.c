/* SPDX-License-Identifier: MIT */
/* The readable TVCROLLDAP against the faithful port's, key by key (make
 * cm-tvc-roll-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable table (src/binding/cm_tvc_roll_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and every key must
 * run readable code. Then the runs: each key from 4,096 states, each one
 * of the port's frozen test states (c64-port/tests/cm_tvc_roll_vectors.h)
 * entered at the key, most stirred further: the gimbal angle and its
 * past value (OGANOW, OGAPAST, often near each other, so the rate is in
 * the phase plane's working range, or at its largest), the error OGA
 * (often within it), 1/CONACC (often small and positive), ROLLFIRE
 * (+0, -0, POSMAX, NEGMAX or any), TEMREG (often +0, -0 or small, and
 * with the jets off often +0),
 * ROLLWORD's pair bits, TIME6, channels 6 and 13, A, L and Q, EBANK (often E6,
 * the file's; now and then any of the eight), any erasable bank's words,
 * the latches, or a state the routines refuse (BBANK not the two banks, a word of 16 bits). Each runs
 * once on each side, as the program's trampoline would call the key's
 * routine, with a hardware record or (now and then) none. Where either
 * port runs, the other must too, with the same memory and io record, word
 * for word; where the readable one refuses, it must leave everything as
 * it found it; and the faithful routine must run in at least 1 of 16 of
 * each key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_tvc_roll_vectors.h"

extern const agc_routine_table cm_tvc_roll_basic, cm_tvc_roll_interpretive;
extern const agc_routine_table faithful_cm_tvc_roll_basic, faithful_cm_tvc_roll_interpretive;
bool cm_tvc_roll_readable(uint16_t key);

/* The addresses of the words the routines use, in E6 (03000 + the
 * address in the bank) or unswitched. */
#define ROLLFIRE_AT 03171u /* 01571: ROLLWORD and TEMREG after it */
#define ROLLWORD_AT 03172u
#define TEMREG_AT   03173u
#define CONACC_AT   03240u /* 01640, 1/CONACC */
#define OGANOW_AT   03262u /* 01662: OGAPAST, OGA, DELOGART, SGNRT, DELOGA, */
#define OGAPAST_AT  03263u /* I and IOGARATE after it */
#define OGA_AT      03264u
#define TIME6_AT    031u

#define CASES 4096u

static uint32_t seed = 984u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned tables_differ(const agc_routine_table *readable, const agc_routine_table *faithful,
                              const char *what)
{
    unsigned i, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL: %s has %u keys, faithful %u\n", what, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < readable->count; ++i)
        if (readable->entries[i].physical != faithful->entries[i].physical) {
            printf("FAIL: %s key %u is %u, faithful %u\n", what, i, readable->entries[i].physical,
                   faithful->entries[i].physical);
            ++failures;
        }
    return failures;
}

/* Every key runs a readable routine. */
static unsigned keys_unwritten(const agc_routine_table *table)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!cm_tvc_roll_readable(key)) {
            printf("FAIL: key %u has no readable routine\n", key);
            ++failures;
        }
    }
    return failures;
}

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A word of a number: often an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 020000u,
                                     057777u, 037777u, 040000u, 010000u, 067777u};
    return rnd() % 4u ? any_word() : edges[rnd() % (sizeof edges / sizeof edges[0])];
}

/* The frozen cases. */
static unsigned mission_cases[CM_TVC_ROLL_VECTORS], mission_count;

/* A sign CCS branches on: often +0, -0, 1 or -1. */
static uint16_t sign_word(void)
{
    unsigned k = rnd() % 8u;

    return k < 2u ? 0u : k == 2u ? 077777u : k == 3u ? 1u : k == 4u ? 077776u : value_word();
}

/* A small signed word, which a limiter passes. */
static uint16_t small_word(void)
{
    uint16_t v = (uint16_t)(rnd() % (rnd() % 2u ? 0100u : 010000u));

    return rnd() % 2u ? v : (uint16_t)(077777u - v);
}

/* A signed word of magnitude below limit. */
static uint16_t signed_below(unsigned limit)
{
    uint16_t v = (uint16_t)((rnd() >> 2) % limit);

    return rnd() % 2u ? v : (uint16_t)(077777u - v);
}

/* A firing state: +0, -0, POSMAX, NEGMAX or any. */
static uint16_t fire_word(void)
{
    static const uint16_t fires[] = {0u, 077777u, 037777u, 040000u};

    return rnd() % 4u ? fires[(rnd() >> 6) % 4u] : any_word();
}

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, unsigned n)
{
    const irc_case *c = &cm_tvc_roll_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) w[3] = rnd() % 4u ? 03000u : 03400u; /* E6, the file's, or E7 */
    w[4] = (uint16_t)((key / 1024u) << 10);
    w[5] = (uint16_t)(02000u + key % 1024u);
    eb = (uint16_t)(w[3] >> 8);
    if (n % 4u != 0u) {
        /* The gimbal angle and its past value, whose difference is the
         * rate (by 16), and the error: mostly in the phase plane's working
         * range (the dead band is 0344, the rate limit 07071) or past the
         * limit, now and then a difference of +-01777, the largest rate, or
         * any. 1/CONACC is now and then tiny, for the shortest firings. */
        w[OGANOW_AT] = value_word();
        k = rnd() % 8u;
        if (k < 4u)
            w[OGAPAST_AT] = (uint16_t)((w[OGANOW_AT] + signed_below(0400u)) & 077777u);
        else if (k == 4u)
            w[OGAPAST_AT] = (uint16_t)((w[OGANOW_AT] + small_word()) & 077777u);
        else if (k == 5u)
            w[OGAPAST_AT] = (uint16_t)((w[OGANOW_AT] + (rnd() % 2u ? 01777u : 076000u)) & 077777u);
        else if (k == 6u)
            w[OGAPAST_AT] = (uint16_t)((w[OGANOW_AT] + signed_below(01000u)) & 077777u);
        else
            w[OGAPAST_AT] = value_word();
        k = rnd() % 4u;
        w[OGA_AT] = k < 2u ? signed_below(02000u) : k == 2u ? small_word() : value_word();
        k = rnd() % 8u;
        w[CONACC_AT] = k < 4u    ? (uint16_t)(rnd() % 010000u)
                       : k == 4u ? (uint16_t)((rnd() >> 6) % 0200u)
                       : k == 5u ? 040000u
                       : k == 6u ? (uint16_t)((rnd() >> 6) % 0100u)
                                 : value_word();
        /* The jets and the firing time. */
        w[ROLLFIRE_AT] = fire_word();
        w[TEMREG_AT] = rnd() % 2u ? sign_word() : small_word();
        /* often, with the jets off, TEMREG reset, so the logic runs */
        if (w[ROLLFIRE_AT] % 077777u == 0u && rnd() % 2u) w[TEMREG_AT] = 0u;
        w[ROLLWORD_AT] = rnd() % 2u ? (uint16_t)((rnd() >> 6) % 4u) : any_word();
        w[TIME6_AT] = rnd() % 2u ? small_word() : any_word();
        io->channel13 = any_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 16u == 0u) w[k] = value_word();
        /* A, L and Q. */
        w[0] = rnd() % 2u ? any_word() : rnd();
        w[1] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 2u) w[2] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 8u == 0u) {
            k = 010u + rnd() % 03770u;
            w[k] = any_word();
        }
        if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 64u == 0u) w[3] = any_word();
        /* now and then another EBANK, any of the eight */
        if (rnd() % 8u == 0u) w[3] = (uint16_t)((rnd() % 8u) << 8);
        if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, n);
            fa = ra = start;
            fio = rio = io;
            memset(&hardware, 0, sizeof hardware);
            hardware.channel6 = (uint16_t)(rnd() & 0377u); /* the jets on before */
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            if (n % 64u != 63u) { /* now and then none */
                fp.hardware = &fhw;
                rp.hardware = &rhw;
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
            else if (sf == EXEC_OK &&
                     (memcmp(&fio, &rio, sizeof fio) || memcmp(&fhw, &rhw, sizeof fhw)))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL key %u case %u: %s\n", key, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL key %u: the faithful routine ran in %u of %u states\n", key, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_TVC_ROLL_VECTORS; ++i)
        if (cm_tvc_roll_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_tvc_roll_basic, &faithful_cm_tvc_roll_basic, "basic");
    failures += tables_differ(&cm_tvc_roll_interpretive, &faithful_cm_tvc_roll_interpretive,
                              "interpretive");
    failures += keys_unwritten(&faithful_cm_tvc_roll_basic);
    if (failures) {
        printf("cm-tvc-roll-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_tvc_roll_basic, &faithful_cm_tvc_roll_basic, &runs);
    printf("cm-tvc-roll-cases: %u of %u keys readable (all basic) from %u states each, %u run by "
           "the faithful routines, %u failures\n",
           cm_tvc_roll_basic.count, faithful_cm_tvc_roll_basic.count, CASES, runs, failures);
    return failures != 0;
}
