/* SPDX-License-Identifier: MIT */
/* The readable RCS-CSM_DAP_EXECUTIVE_PROGRAMS against the faithful port's,
 * key by key (make cm-rcs-dap-exec-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable table (src/binding/cm_rcs_dap_exec_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and every key must
 * run readable code. Then the runs: each key from 4,096 states, each one
 * of the port's frozen test states (c64-port/tests/cm_rcs_dap_exec_vectors.h)
 * entered at the key, most stirred further: FLAGWRD6's bits 15 and 14, the
 * CDU angles (often at an edge), A (the sine or cosine a key comes back
 * with, often holding an overflow, which makes the TS after it skip), L
 * and Q, CAPSI and the AMGB words, EBANK (often E6, KMPAC's), any erasable
 * bank's words, the latches, or a state the routines refuse (BBANK not
 * the two banks, an EBANK past E7, a word of 16 bits). Each runs once on each side, as the
 * program's trampoline would call the key's routine, with a hardware
 * record or (now and then) none. Where either port runs, the other must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it; and the
 * faithful routine must run in at least 1 of 16 of each key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_rcs_dap_exec_vectors.h"

extern const agc_routine_table cm_rcs_dap_exec_basic, cm_rcs_dap_exec_interpretive;
extern const agc_routine_table faithful_cm_rcs_dap_exec_basic,
    faithful_cm_rcs_dap_exec_interpretive;
bool cm_rcs_dap_exec_readable(uint16_t key);

/* The addresses of the words the routines use, unswitched or in E6
 * (03000 + the address in the bank). */
#define CDUX_AT     032u
#define CDUZ_AT     034u
#define FLAGWRD6_AT 0102u
/* AMGB1, AMGB4, AMGB5, AMGB7, AMGB8 and CAPSI, 01640-01645. */
#define AMGB1_AT 03240u

#define CASES 4096u

static uint32_t seed = 1037u;
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
        if (!cm_rcs_dap_exec_readable(key)) {
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
static unsigned mission_cases[CM_RCS_DAP_EXEC_VECTORS], mission_count;

/* A: any word, often one with an overflow (bits 16 and 15 differ). */
static uint16_t a_word(void)
{
    unsigned k = rnd() % 4u;

    return k == 0u
               ? (uint16_t)(rnd() % 2u ? 040000u | (rnd() & 037777u) : 0100000u | (rnd() & 037777u))
           : k == 1u ? value_word()
           : k == 2u ? (uint16_t)(value_word() | ((value_word() & 040000u) << 1))
                     : rnd();
}

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, unsigned n)
{
    const irc_case *c = &cm_rcs_dap_exec_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) w[3] = rnd() % 4u ? 03000u : 03400u; /* E6, KMPAC's, or E7 */
    w[4] = (uint16_t)((key / 1024u) << 10);
    w[5] = (uint16_t)(02000u + key % 1024u);
    eb = (uint16_t)(w[3] >> 8);
    if (n % 4u != 0u) {
        /* FLAGWRD6's bits 15 and 14, which say whether the RCS DAP runs. */
        k = rnd() % 4u;
        w[FLAGWRD6_AT] = (uint16_t)(k == 0u ? any_word() : (rnd() & 017777u) | (k << 13));
        if (rnd() % 16u == 0u) w[FLAGWRD6_AT] = rnd() % 2u ? 0u : 077777u;
        /* The CDU angles, and the words the job writes. */
        w[CDUX_AT] = value_word();
        w[CDUZ_AT] = value_word();
        for (k = 0; k < 6u; ++k)
            if (rnd() % 2u) w[AMGB1_AT + k] = value_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A, L and Q. */
        w[0] = a_word();
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
        if (rnd() % 8u == 0u) w[3] = (uint16_t)((rnd() % 8u) << 8); /* any of the eight */
        if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
        /* Now and then an EBANK past E7, with BBANK agreeing. */
        if (rnd() % 64u == 0u) {
            w[3] = (uint16_t)((rnd() % 0100u) << 8);
            eb = (uint16_t)(w[3] >> 8);
        }
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
    unsigned i, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_RCS_DAP_EXEC_VECTORS; ++i)
        if (cm_rcs_dap_exec_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_rcs_dap_exec_basic, &faithful_cm_rcs_dap_exec_basic, "basic");
    failures += tables_differ(&cm_rcs_dap_exec_interpretive, &faithful_cm_rcs_dap_exec_interpretive,
                              "interpretive");
    failures += keys_unwritten(&faithful_cm_rcs_dap_exec_basic);
    if (failures) {
        printf("cm-rcs-dap-exec-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_rcs_dap_exec_basic, &faithful_cm_rcs_dap_exec_basic, &runs);
    printf(
        "cm-rcs-dap-exec-cases: %u of %u keys readable (all basic) from %u states each, %u run by "
        "the faithful routines, %u failures\n",
        cm_rcs_dap_exec_basic.count, faithful_cm_rcs_dap_exec_basic.count, CASES, runs, failures);
    return failures != 0;
}
