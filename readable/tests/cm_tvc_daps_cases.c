/* SPDX-License-Identifier: MIT */
/* The readable TVCDAPS against the faithful port's, key by key (make
 * cm-tvc-daps-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable table (src/binding/cm_tvc_daps_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and every key must
 * run readable code. Then the runs: each key from 4,096 states, each one
 * of the port's frozen test states (c64-port/tests/cm_tvc_daps_vectors.h)
 * entered at the key, most stirred further: STROKER, DAPDATR1's LM bits,
 * T5TVCDT, the CDUs near their past values or not, the errors, filter
 * nodes, coefficients and gain (often small, so that the limiters both
 * take and pass; now and then none, with the offsets one under the
 * actuator limit and a low word that rounds them up to it), channel 14, A, L and Q (often a key of the file, a word
 * one of its subroutines returns to, or 3, for a copycycle's TC Q),
 * EBANK (often E6, the file's), any erasable bank's words, the latches,
 * or a state the routines refuse (BBANK not the two banks, a word of 16
 * bits). Each runs once on each side, as the program's trampoline would
 * call the key's routine, with a hardware record or (now and then) none.
 * Where either port runs, the other must too, with the same memory and io
 * record, word for word; where the readable one refuses, it must leave
 * everything as it found it; and the faithful routine must run in at
 * least 1 of 16 of each key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_tvc_daps_vectors.h"

extern const agc_routine_table cm_tvc_daps_basic, cm_tvc_daps_interpretive;
extern const agc_routine_table faithful_cm_tvc_daps_basic, faithful_cm_tvc_daps_interpretive;
bool cm_tvc_daps_readable(uint16_t key);

/* The addresses of the words the routines use, in E6 (03000 + the
 * address in the bank). */
#define STROKER_AT  03174u /* 01574 */
#define DAPDATR1_AT 03066u /* 01466 */
#define T5TVCDT_AT  03225u /* 01625 */
#define TVCPHASE_AT 03244u /* 01644, PCDUYPST and PCDUZPST after it */
#define DAP_FIRST   03075u /* 01475 to 01721: the DAPs' own words */
#define DAP_LAST    03321u
#define N10_AT      03323u /* 01723: N10 to D32, 15 words */
#define VARK_AT     03241u /* 01641 */
#define PDELOFF_AT  03205u /* 01605, YDELOFF after it */
#define CDUY        033u   /* CDUZ after it */
/* Words a copycycle's TC Q may name: the file's keys, the words its
 * subroutines return to, a word in another bank's range, fixed-fixed
 * memory, and EBANK (3). */
static const uint16_t targets[] = {02530u, 02541u, 02543u, 02675u, 02732u, 02743u, 02745u, 03051u,
                                   03107u, 02550u, 02556u, 02620u, 02632u, 02636u, 02656u, 02673u,
                                   02774u, 03006u, 03012u, 03032u, 03047u, 03243u, 04701u, 3u};

#define CASES 4096u

static uint32_t seed = 961u;
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
static unsigned keys_unwritten(const agc_routine_table *table, unsigned *readable)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!cm_tvc_daps_readable(key)) {
            printf("FAIL: key %u has no readable routine\n", key);
            ++failures;
        } else {
            ++*readable;
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
static unsigned mission_cases[CM_TVC_DAPS_VECTORS], mission_count;

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

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, unsigned n)
{
    const irc_case *c = &cm_tvc_daps_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
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
        /* The DAPs' words: errors, nodes, commands, offsets. */
        for (k = DAP_FIRST; k <= DAP_LAST; ++k)
            if (rnd() % 2u) w[k] = rnd() % 2u ? small_word() : value_word();
        /* The stroke test, the LM, the T5 time. */
        w[STROKER_AT] = sign_word();
        k = rnd() % 4u;
        w[DAPDATR1_AT] = (uint16_t)((k & 1u ? 010000u : 0u) | (k & 2u ? 020000u : 0u) |
                                    (rnd() % 4u ? 0u : any_word() & 047777u));
        w[T5TVCDT_AT] = value_word();
        /* The CDUs, near their past values or not. */
        for (k = 0; k < 2u; ++k) {
            w[CDUY + k] = any_word();
            w[TVCPHASE_AT + 1u + k] =
                rnd() % 2u ? (uint16_t)((w[CDUY + k] + small_word()) & 077777u) : any_word();
        }
        /* The coefficients and gain. */
        for (k = 0; k < 15u; ++k) w[N10_AT + k] = rnd() % 2u ? small_word() : value_word();
        w[VARK_AT] = rnd() % 2u ? small_word() : value_word();
        w[TVCPHASE_AT] = sign_word();
        io->channel14 = any_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 16u == 0u) w[k] = value_word();
        /* Now and then no gain, so each command is its offset alone,
         * one under the actuator limit ACTSAT (253) with a low word whose
         * DOUBLE overflows: ACTLIM's round up takes it to the limit. */
        if (rnd() % 8u == 0u) {
            bool minus = rnd() % 2u;

            w[VARK_AT] = 0u;
            for (k = 0; k < 4u; k += 2u) {
                uint16_t low = (uint16_t)(020000u + rnd() % 020000u);
                w[PDELOFF_AT + k] = minus ? 077777u - 0374u : 0374u; /* 252 */
                w[PDELOFF_AT + k + 1u] = minus ? (uint16_t)(077777u - low) : low;
            }
        }
        /* A, L and Q. */
        w[0] = rnd() % 2u ? any_word() : rnd();
        w[1] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 2u)
            w[2] = rnd() % 2u   ? targets[rnd() % (sizeof targets / sizeof targets[0])]
                   : rnd() % 2u ? any_word()
                                : rnd();
        if (rnd() % 8u == 0u) {
            k = 010u + rnd() % 03770u;
            w[k] = any_word();
        }
        if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 64u == 0u) w[3] = any_word();
        if (rnd() % 8u == 0u) w[3] = (uint16_t)((rnd() % 8u) << 8); /* any of the eight */
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
    unsigned readable_keys = 0;
    unsigned i, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_TVC_DAPS_VECTORS; ++i)
        if (cm_tvc_daps_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_tvc_daps_basic, &faithful_cm_tvc_daps_basic, "basic");
    failures += tables_differ(&cm_tvc_daps_interpretive, &faithful_cm_tvc_daps_interpretive,
                              "interpretive");
    failures += keys_unwritten(&faithful_cm_tvc_daps_basic, &readable_keys);
    if (failures) {
        printf("cm-tvc-daps-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_tvc_daps_basic, &faithful_cm_tvc_daps_basic, &runs);
    printf("cm-tvc-daps-cases: %u of %u keys readable (all basic) from %u states each, %u run by "
           "the faithful routines, %u failures\n",
           readable_keys, faithful_cm_tvc_daps_basic.count, CASES, runs, failures);
    return failures != 0;
}
