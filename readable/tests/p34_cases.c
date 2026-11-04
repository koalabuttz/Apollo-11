/* SPDX-License-Identifier: MIT */
/* The readable P34-35_P74-75 against the faithful port's, key by key
 * (make p34-cases ROPE_LM ROPE_CM), one mission per build.
 *
 * Built with the readable tables (src/binding/p34_routines.c) and the
 * faithful port's (c64-port/src/lm_p34_p35.c or cm_p34_p35.c) under the
 * same names with faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p34_p35/), written
 * in the listing's notation, must be the mission's rope, word for word.
 * Then the keys, which must be the faithful tables', and the keys the
 * parts written so far hold must run readable code, the others the
 * faithful port's. Then the runs: each readable key from 4,096 states
 * (each other key from 16), each one of the port's frozen test states
 * (c64-port/tests/lm_p34_p35_vectors.h or cm_p34_p35_vectors.h) entered
 * at the key, most stirred further: MPAC and its mode, the push-down list
 * and its pointer, the index registers, A (now and then with overflow),
 * Q, the flag words (often one bit changed), the vectors and times (any
 * erasable bank's words), NEWJOB, the overflow indicator, now and then
 * any word but the interpreter's own, the latches, or a state the
 * routines refuse (another work area, BBANK not the two banks, a word of
 * 16 bits). Each runs once on each side, as the program's trampoline
 * would call the key's routine. Where either port runs, the other must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it; and the
 * faithful routine must run from at least 1 in 16 of the states of every
 * readable key. A key the binding hands to the faithful port must give
 * the faithful routine's result, refusals and all. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#if LUMINARY099
#include "lm_p34_p35_vectors.h"
#else
#include "cm_p34_p35_vectors.h"
#endif

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code P34_TTPI_CODE, P34_SWCHCLR_CODE, P34_P74D_CODE, P34_P74E_CODE,
    P34_N58_CODE, P35_CODE, S33_34_1_CODE, FIGTIME_CODE, OKMAX_CODE, S34_35_1_CODE, S34_35_2_CODE,
    S34_35_3_CODE, LOMAT_CODE, S34_35_5_CODE, S34_35_5_LOADED_CODE, FLAGOFF_CODE, VN1645_CODE,
    GETMGA_CODE, CLUPDATE_CODE;
#if !LUMINARY099
extern const struct interpretive_code P34_NN_CODE;
#endif
static const struct interpretive_code *const codes[] = {
#if !LUMINARY099
    &P34_NN_CODE,
#endif
    &P34_TTPI_CODE, &P34_SWCHCLR_CODE, &P34_P74D_CODE, &P34_P74E_CODE, &P34_N58_CODE,
    &P35_CODE,      &S33_34_1_CODE,    &FIGTIME_CODE,  &OKMAX_CODE,    &S34_35_1_CODE,
    &S34_35_2_CODE, &S34_35_3_CODE,    &LOMAT_CODE,    &S34_35_5_CODE, &S34_35_5_LOADED_CODE,
    &FLAGOFF_CODE,  &VN1645_CODE,      &GETMGA_CODE,   &CLUPDATE_CODE,
};
#if LUMINARY099
#define readable_basic_table        lm_p34_p35_basic
#define readable_interpretive_table lm_p34_p35_interpretive
#define faithful_basic              faithful_lm_p34_p35_basic
#define faithful_interpretive       faithful_lm_p34_p35_interpretive
#define frozen_cases                lm_p34_p35_cases
#define FROZEN_CASES                LM_P34_P35_VECTORS
#define MISSION_IS_CM               0u
#define ROPE                        0u
#define MISSION                     "LM"
#define NEWOPS_AT                   06070u
#define NORMEX_AT                   03463u
#define RETURN_WORDS 03463u, 03470u, 03464u, 03465u /* NORMEX, SUBEXIT, QSAVED, RTRN */
typedef p66_bias_io mission_io;
/* The parts written so far: part 1, P34 and P74 (35,2545-2724), and
 * DISPLAYE, P3XORP7X and VNPOOH (35,3674-3722); part 2, P35, P75 and
 * S33/34.1 (35,2725-3273); part 3, S34/35.1 to VN1645 and CLUPDATE
 * (35,3274-3673). */
#define WRITTEN                                                                                    \
    {035u, 02545u, 02724u}, {035u, 03674u, 03722u}, {035u, 02725u, 03273u}, {035u, 03274u, 03673u}
#else
#define readable_basic_table        cm_p34_p35_basic
#define readable_interpretive_table cm_p34_p35_interpretive
#define faithful_basic              faithful_cm_p34_p35_basic
#define faithful_interpretive       faithful_cm_p34_p35_interpretive
#define frozen_cases                cm_p34_p35_cases
#define FROZEN_CASES                CM_P34_P35_VECTORS
#define MISSION_IS_CM               1u
#define ROPE                        1u
#define MISSION                     "CM"
#define NEWOPS_AT                   06037u
#define NORMEX_AT                   01340u
#define RETURN_WORDS 01340u, 02370u, 02366u, 02367u /* NORMEX, SUBEXIT, QSAVED, RTRN */
typedef cm_native_io mission_io;
/* The parts written so far: part 1, P34 and P74 (35,2000-2156), and
 * DISPLAYE and P3XORP7X (35,3070-3105) and VNPOOH (02,2575-2613); part 2,
 * P35, P75 and S33/34.1 (35,2157-2525); part 3, S34/35.2 to VN1645 and
 * CLUPDATE (35,2526-3067), and S34/35.1 (36,2000-2012). */
#define WRITTEN                                                                                    \
    {035u, 02000u, 02156u}, {035u, 03070u, 03105u}, {02u, 02575u, 02613u}, {035u, 02157u, 02525u}, \
        {035u, 02526u, 03067u}, {036u, 02000u, 02012u}
#endif
extern const agc_routine_table readable_basic_table, readable_interpretive_table;
extern const agc_routine_table faithful_basic, faithful_interpretive;
bool p34_p35_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define X1_AT       0046u /* X1, X2, S1 and S2, in the work area */
#define EDOP_AT     0023u
#define NEWJOB_AT   0067u
#define FLAGWRD0_AT 0074u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define OVFIND_AT   0121u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 3435u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned words_differ(const struct interpretive_code *code)
{
    unsigned i, failures = 0;

    for (i = 0; i < code->count; ++i) {
        agc_word rope = 0;
        unsigned physical = code->bank * 1024u + code->first - 02000u + i;
        if (!apollo_rope_read(ROPE, (uint16_t)physical, &rope) || rope != code->words[i]) {
            printf("FAIL: %02o,%04o is %05o, the rope's %05o\n", code->bank, code->first + i,
                   code->words[i], rope);
            ++failures;
        }
    }
    return failures;
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

/* Whether a key is in the parts written so far (its bank, and its
 * address in 02000-03777, fixed-fixed ones too). */
static bool in_written_part(uint16_t key)
{
    static const struct {
        unsigned bank, first, last;
    } written[] = {WRITTEN};
    unsigned z = 02000u + key % 1024u, i;

    for (i = 0; i < sizeof written / sizeof written[0]; ++i)
        if (key / 1024u == written[i].bank && z >= written[i].first && z <= written[i].last)
            return true;
    return false;
}

static unsigned parts_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (in_written_part(key) != p34_p35_readable(key, order_pair)) {
            printf("FAIL: %02o,%04o %s readable\n", key / 1024u, 02000u + key % 1024u,
                   in_written_part(key) ? "is not" : "is");
            ++failures;
        }
    }
    return failures;
}

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A word of a vector or a DP number: often an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 020000u,
                                     057777u, 037777u, 040000u, 010000u, 067777u};
    return rnd() % 4u ? any_word() : edges[rnd() % COUNT(edges)];
}

/* The frozen cases. */
static unsigned mission_cases[FROZEN_CASES], mission_count;

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, mission_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &frozen_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;
    static const uint16_t return_words[] = {RETURN_WORDS};
    uint16_t *w = s->words, fixloc;
    unsigned i, k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    /* Bank 35 wants the superbank bit clear. */
    if (rnd() % 4u) io->superbank &= (uint16_t)~0100u;
    eb = (uint16_t)(w[3] >> 8);
    w[4] = (uint16_t)(bank << 10);
    if (interpretive) {
        w[5] = NEWOPS_AT;
        w[LOC_AT] = z;
        w[INTBIT15_AT] = (uint16_t)(w[4] & 040000u);
        w[EDOP_AT] = 0u; /* as DANZIG leaves it before every NEWOPS */
    } else {
        w[5] = z;
    }
    fixloc = w[FIXLOC_AT];
    if (n % 4u != 0u) {
        fixloc = w[FIXLOC_AT] = (uint16_t)(0401u + 054u * (rnd() % 5u));
        for (k = 0; k < 7u; ++k) w[MPAC_AT + k] = value_word();
        k = rnd() % 3u;
        w[MODE_AT] = k == 0u ? 0u : k == 1u ? 1u : 077776u; /* DP, TP or vector */
        for (k = 0; k < 040u; ++k)
            if (rnd() % 2u) w[(fixloc + k) & 03777u] = value_word();
        /* The flag words, often one bit changed. */
        for (k = 0; k < 12u; ++k) {
            if (rnd() % 4u == 0u)
                w[FLAGWRD0_AT + k] = any_word();
            else if (rnd() % 3u == 0u)
                w[FLAGWRD0_AT + k] ^= (uint16_t)(1u << (rnd() % 15u));
        }
        /* The vectors and times: any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A (now and then with overflow), Q. */
        k = rnd() % 4u;
        w[0] = k < 2u ? any_word() : k == 2u ? rnd() : (uint16_t)(rnd() % 2u ? 040000u : 0137777u);
        if (rnd() % 2u) w[2] = any_word();
        /* The index registers. */
        if (rnd() % 2u) w[(fixloc + X1_AT + rnd() % 4u) & 03777u] = any_word();
        k = rnd() % 8u;
        w[NEWJOB_AT] = k < 5u    ? (uint16_t)(rnd() % 8u * 12u)
                       : k == 5u ? 077777u /* -0 */
                                 : (uint16_t)(rnd() & 077777u);
        w[OVFIND_AT] = rnd() % 4u ? 0u : rnd() % 2u ? 1u : 077776u;
        /* Any other word, not the interpreter's own (MPAC to PUSHLOC: LOC
         * is the key, MODE one of three; EDOP, +0 at a pair's start) nor
         * FIXLOC. */
        if (rnd() % 8u == 0u) {
            k = 010u + rnd() % 03770u;
            if ((k < 0154u || k > 0166u) && k != FIXLOC_AT && k != EDOP_AT) w[k] = any_word();
        }
        if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 32u == 0u) w[INTBIT15_AT] = any_word();
        if (rnd() % 64u == 0u) w[3] = any_word();
        eb = (uint16_t)(w[3] >> 8);
    }
    /* Now and then a work area the routines do not take: a NOVAC job's
     * (FAKEPRET, MPAC -36D), one a word off a VAC area, or none. */
    if (n % 4u != 0u && rnd() % 64u == 0u) {
        static const uint16_t near[] = {0400u, 0402u, 0430u, 0662u, 0735u};
        k = rnd() % 3u;
        fixloc = w[FIXLOC_AT] = k == 0u   ? (uint16_t)(MPAC_AT - 044u)
                                : k == 1u ? near[rnd() % COUNT(near)]
                                          : (uint16_t)(rnd() % 02000u);
    }
    /* The push-down list in the work area. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* The words the interpretive code returns through by GOTO (NORMEX,
     * SUBEXIT, QSAVED, RTRN; the LM's in E7), each a fixed CADR (as STQ
     * leaves it): a chain of erasable CADRs that never reaches a fixed one
     * loops in the AGC until the night watchman restarts it, and the
     * faithful port's host build stops the test there. Half of NORMEX's in
     * 4000-7777, where DISPLAYE's TC NORMEX (the CM's, at the same
     * address) takes them. */
    if (rnd() % 2u) w[NORMEX_AT] = (uint16_t)(04000u + rnd() % 04000u);
    for (i = 0; i < COUNT(return_words); ++i)
        if (!(w[return_words[i]] & 074000u))
            w[return_words[i]] = (uint16_t)(04000u + rnd() % 074000u);
    /* The interrupt latch either way now and then. */
    if (n % 4u != 0u && rnd() % 8u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 2u);
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        bool written = p34_p35_readable(key, interpretive != 0);
        unsigned ran = 0, bad = 0, cases = written ? CASES : OTHER_CASES;

        *readable_keys += written;
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            sf = faithful->entries[i].call(&p, &fa, &fio);
            sr = readable->entries[i].call(&p, &ra, &rio);
            ran += sf == EXEC_OK;
            if (!written) { /* the faithful routine on both sides, refusals and all */
                if (sf != sr || memcmp(&fa, &ra, sizeof fa) || memcmp(&fio, &rio, sizeof fio))
                    why = "the table does not run the faithful routine";
            } else if (sr != EXEC_OK &&
                       (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
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
        /* Each readable key's faithful routine must run in at least 1 of
         * 16 states, so that a change to the states cannot quietly leave a
         * key with a handful of live ones. (The states are tuned to a
         * part's keys when it is written.) */
        if (written && ran < cases / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states (fewer than 1 in "
                   "16)\n",
                   key / 1024u, z, ran, cases);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0, readable_basic = 0, readable_pairs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < FROZEN_CASES; ++i)
        if (frozen_cases[i].cm == MISSION_IS_CM) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&readable_basic_table, &faithful_basic, "basic");
    failures += tables_differ(&readable_interpretive_table, &faithful_interpretive, "interpretive");
    failures += parts_unwritten(&faithful_basic, false);
    failures += parts_unwritten(&faithful_interpretive, true);
    if (failures || !mission_count) {
        printf("p34-cases (" MISSION "): %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&readable_basic_table, &faithful_basic, 0, &runs, &readable_basic);
    failures +=
        keys(&readable_interpretive_table, &faithful_interpretive, 1, &runs, &readable_pairs);
    printf("p34-cases (" MISSION
           "): %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each (the others from %u), %u run by the faithful routines, "
           "%u failures\n",
           words, readable_basic + readable_pairs,
           readable_basic_table.count + readable_interpretive_table.count, readable_basic,
           readable_pairs, CASES, OTHER_CASES, runs, failures);
    return failures != 0;
}
