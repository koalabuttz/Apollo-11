/* SPDX-License-Identifier: MIT */
/* The readable radar sampling loop against the faithful port's, key by
 * key (make radar-leadin-cases ROPE_LM ROPE_CM). LM only.
 *
 * Built with the readable table (src/binding/radar_leadin_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the keys, which must be the faithful table's. Then the runs: each
 * key from 16,384 states, each one of the port's frozen test states
 * (c64-port/tests/lm_radar_leadin_routines_vectors.h) entered at the key,
 * most stirred further: EBANK 4 (as the routines run), the sampling
 * period RSAMPDT (positive, +0, -0, negative or any), RTSTLOC and RTSTMAX
 * (the double words of RSTACK, equal now and then, or any: one that takes
 * INDEX RTSTLOC out of erasable memory, or the TS that cycles it into an
 * overflow), RTSTBASE (0 or 2, or any), RTSTDEX (0 to 5, -0, a little
 * below the table, into erasable memory, or any), RFAILCNT, FLAGWRD5's
 * R77FLAG, SAMPLSUM and RSTACK, A, L and Q, now and then any erasable
 * word, the latches, or a state the routines refuse (another FB, BBANK not
 * the two banks, a word of 16 bits). Each runs once on each side, as the
 * program's trampoline would call the key's routine. Where either port
 * runs, the other must too, with the same memory and io record, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run from some state of every
 * key. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "lm_radar_leadin_routines_vectors.h"

extern const agc_routine_table lm_radar_leadin_routines_basic,
    lm_radar_leadin_routines_interpretive;
extern const agc_routine_table faithful_lm_radar_leadin_routines_basic,
    faithful_lm_radar_leadin_routines_interpretive;

/* The physical addresses of the words the routines use, in E-bank 4
 * (EBANK=RSTACK), and FLAGWRD5 (unswitched). */
#define EBANK4_AT(a) (uint16_t)(02000u + ((a) - 01400u))
#define RSAMPDT_AT   EBANK4_AT(01755u)
#define RTSTLOC_AT   EBANK4_AT(01754u)
#define RFAILCNT_AT  EBANK4_AT(01756u)
#define RTSTDEX_AT   EBANK4_AT(01751u)
#define RTSTBASE_AT  EBANK4_AT(01753u)
#define RTSTMAX_AT   EBANK4_AT(01752u)
#define RSTACK_AT    EBANK4_AT(01600u)
#define SAMPLSUM_AT  01101u
#define FLAGWRD5_AT  0101u
#define BUF2_AT      0133u

#define CASES 16384u

static uint32_t seed = 49025u;
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

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A word now and then an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 020000u,
                                     057777u, 037777u, 040000u, 010000u, 067777u};
    return rnd() % 4u ? any_word() : edges[rnd() % (sizeof edges / sizeof edges[0])];
}

/* A small number, now and then negative (one's complement), or any. */
static uint16_t small_word(unsigned below)
{
    unsigned k = rnd() % 8u;
    if (k < 5u) return (uint16_t)(rnd() % below);
    if (k == 5u) return 077777u;                              /* -0 */
    if (k == 6u) return (uint16_t)(077777u - rnd() % 01200u); /* a little below */
    return value_word();
}

/* Indexes that reach the edges: from RSTACK (01600) and RDRLOCS (02054),
 * as one's complement words. */
static const uint16_t dxch_edges[] = {076204u, 076205u, 076206u, 0176u, 0177u};
static const uint16_t caf_edges[] = {075723u, 075725u, 075727u, 075730u,
                                     075732u, 075733u, 077776u, 6u};
static uint16_t edge(const uint16_t *edges, size_t size)
{
    return edges[rnd() % (size / sizeof edges[0])];
}

/* One state at the key (its physical address): a frozen case's, and for
 * most n more. */
static void state(apollo_overlay_state *s, p66_bias_io *io, uint16_t key, unsigned n)
{
    const irc_case *c =
        &lm_radar_leadin_routines_cases[(n * 7u + rnd()) % LM_RADAR_LEADIN_ROUTINES_VECTORS];
    uint16_t *w = s->words;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    w[4] = 052000u; /* FB 25 */
    w[5] = (uint16_t)(02000u + key % 1024u);
    if (n % 4u != 0u) {
        w[3] = rnd() % 8u ? 02000u : (uint16_t)((rnd() % 8u) << 8); /* EBANK */
        k = rnd() % 6u;
        w[RSAMPDT_AT] = k < 3u    ? (uint16_t)(1u + rnd() % 200u)
                        : k == 3u ? 0u
                        : k == 4u ? 077777u
                                  : value_word();
        w[RTSTLOC_AT] = rnd() % 4u ? (uint16_t)(2u * (rnd() % 4u)) : small_word(010u);
        if (rnd() % 8u == 0u) w[RTSTLOC_AT] = value_word();
        /* Now and then one that takes INDEX RTSTLOC; DXCH RSTACK to the
         * edges of what the routines take: Q, ZERO, 01776, 01777. */
        if (rnd() % 8u == 0u) w[RTSTLOC_AT] = edge(dxch_edges, sizeof dxch_edges);
        w[RTSTMAX_AT] = rnd() % 4u ? 6u : rnd() % 2u ? w[RTSTLOC_AT] : value_word();
        w[RTSTBASE_AT] = rnd() % 4u ? (uint16_t)(2u * (rnd() % 2u)) : value_word();
        w[RTSTDEX_AT] = small_word(6u);
        /* Now and then one that takes INDEX RTSTDEX; CAF RDRLOCS to A, Q,
         * Z, ZERO, 010 or the edges of the table. */
        if (rnd() % 8u == 0u) w[RTSTDEX_AT] = edge(caf_edges, sizeof caf_edges);
        if (rnd() % 4u == 0u) w[RFAILCNT_AT] = value_word();
        k = rnd() % 4u;
        if (k == 0u)
            w[FLAGWRD5_AT] ^= 02000u; /* R77FLAG */
        else if (k == 1u)
            w[FLAGWRD5_AT] = any_word();
        for (k = 0; k < 010u; ++k)
            if (rnd() % 2u) w[RSTACK_AT + k] = value_word();
        w[SAMPLSUM_AT] = value_word();
        w[SAMPLSUM_AT + 1u] = value_word();
        if (rnd() % 2u) w[BUF2_AT] = any_word();
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
    }
    w[6] = (uint16_t)(w[4] | (w[3] >> 8));
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            p66_bias_io io, fio, rio;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, n);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            sf = faithful->entries[i].call(&p, &fa, &fio);
            sr = readable->entries[i].call(&p, &ra, &rio);
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
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %02o,%04o case %u: %s\n", key / 1024u, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine never ran\n", key / 1024u, z);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures += tables_differ(&lm_radar_leadin_routines_basic,
                              &faithful_lm_radar_leadin_routines_basic, "basic");
    failures += tables_differ(&lm_radar_leadin_routines_interpretive,
                              &faithful_lm_radar_leadin_routines_interpretive, "interpretive");
    if (failures) {
        printf("radar-leadin-cases: %u failures\n", failures);
        return 1;
    }
    failures +=
        keys(&lm_radar_leadin_routines_basic, &faithful_lm_radar_leadin_routines_basic, &runs);
    printf("radar-leadin-cases: %u keys from %u states each, %u run by the faithful routines, "
           "%u failures\n",
           lm_radar_leadin_routines_basic.count, CASES, runs, failures);
    return failures != 0;
}
