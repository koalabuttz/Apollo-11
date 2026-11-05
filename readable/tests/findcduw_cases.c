/* SPDX-License-Identifier: MIT */
/* The readable FINDCDUW--GUIDAP_INTERFACE against the faithful port's, key
 * by key (make findcduw-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/findcduw_routines.c) and the
 * faithful port's (c64-port/src/lm_findcduw.c) under the same names with
 * faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/findcduw/),
 * written in the listing's notation, must be the LM's rope, word for word.
 * Then the keys, which must be the faithful tables', each running readable
 * code: the file is whole. Then the runs: each key from 4,096 states, each
 * one of the port's frozen test states (c64-port/tests/findcduw_vectors.h)
 * entered at the key, most stirred further: MPAC and its mode, the
 * push-down list and its pointer, the index registers, A (now and then
 * with overflow), Q, the flag words, the vectors (any erasable bank's
 * words), NEWJOB, the overflow indicator, now and then any word but the
 * interpreter's own, the latches, channels 13, 30 and 31, no hardware
 * record, or a state the routines refuse (another work area, BBANK not
 * the two banks, a word of 16 bits); and the words the file's routines
 * test: DAPBOOLS's CSMDOCKD and XOVINHIB bits, the CDUs and the commanded
 * ones, NDXCDUW (mostly 0 or 1), FLPAUTNO and FLAGOODW (each sign),
 * -DELGMB, the returns kept in TEM2 and TEM4 (mostly words of bank 30)
 * or CDUWXFR's count in TEM2, TEM3, the CDUs QUICTRIG reads (CDUSPOT),
 * and now and then a zero command (in MPAC or UNWC/2). Each runs once on
 * each side, as the program's trampoline would call the key's routine.
 * Where either port runs, the other must too, with the same memory, io
 * record and channels, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * from at least 1 in 16 of the states of every key. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "findcduw_vectors.h"

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code INITCDUW_CODE, FINDCDUW_CODE, PAUTNO_CODE, AFTRFLTR_CODE,
    TCQCDUW_CODE, UNWCTEST_CODE, NB2CDUSP_CODE, NB2CDUSP_RVQ_CODE;
static const struct interpretive_code *const codes[] = {
    &INITCDUW_CODE, &FINDCDUW_CODE, &PAUTNO_CODE,   &AFTRFLTR_CODE,
    &TCQCDUW_CODE,  &UNWCTEST_CODE, &NB2CDUSP_CODE, &NB2CDUSP_RVQ_CODE};
#define readable_basic_table        lm_findcduw_basic
#define readable_interpretive_table lm_findcduw_interpretive
#define faithful_basic              faithful_lm_findcduw_basic
#define faithful_interpretive       faithful_lm_findcduw_interpretive
#define frozen_cases                findcduw_cases
#define FROZEN_CASES                FINDCDUW_VECTORS
#define MISSION_IS_CM               0u
#define ROPE                        0u
#define NEWOPS_AT                   06070u
extern const agc_routine_table readable_basic_table, readable_interpretive_table;
extern const agc_routine_table faithful_basic, faithful_interpretive;
bool findcduw_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use (E-bank 6's by their
 * physical address). */
#define X1_AT       0046u /* X1, X2, S1 and S2, in the work area */
#define EDOP_AT     0023u
#define NEWJOB_AT   0067u
#define FLAGWRD0_AT 0074u
#define DAPBOOLS_AT 0111u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define OVFIND_AT   0121u
#define TEM2_AT     0142u /* TEM1 to TEM5 */
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define CDUX_AT     0032u /* CDUX, CDUY, CDUZ */
#define CDUSPOT_AT  0766u /* CDUSPOT to CDUSPOT +5 */
#define CDUXD_AT    03235u
#define NDXCDUW_AT  03250u
#define FLAGOODW_AT 03251u
#define FLPAUTNO_AT 03252u
#define MDELGMB_AT  03275u /* -DELGMB to -DELGMB +2 */
#define UNWC_AT     03261u /* UNWC/2 */
#define CSMDOCKD    010000u
#define XOVINHIB    000400u

#define CASES 4096u

static uint32_t seed = 3113u;
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

/* The file is whole: every key of a faithful table runs readable code. */
static unsigned unreadable(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!findcduw_readable(key, order_pair)) {
            printf("FAIL: %02o,%04o is not readable\n", key / 1024u, 02000u + key % 1024u);
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

/* A word of each sign now and then: +0, -0, 1, -1, or any. */
static uint16_t edge_or_any(void)
{
    static const uint16_t edges[] = {0u, 077777u, 1u, 077776u};
    return rnd() % 2u ? edges[rnd() % COUNT(edges)] : any_word();
}

/* The frozen cases. */
static unsigned mission_cases[FROZEN_CASES], mission_count;

/* A return kept in TEM2 or TEM4, or in Q: mostly a word of bank 30's
 * window (02000-07777), now and then a small one or any. */
static uint16_t return_word(void)
{
    unsigned k = rnd() % 8u;
    return k < 6u    ? (uint16_t)(02000u + rnd() % 06000u)
           : k == 6u ? (uint16_t)(rnd() % 020u)
                     : any_word();
}

/* A flag of FINDCDUW's: +0, the bit, -0, negative, or any. */
static uint16_t flag_word(uint16_t bit)
{
    static const uint16_t others[] = {0u, 077777u, 070000u, 1u};
    unsigned k = rnd() % 6u;
    return k < 2u ? 0u : k < 4u ? bit : k == 4u ? others[rnd() % COUNT(others)] : any_word();
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, lm_channel_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &frozen_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) io->io.superbank &= (uint16_t)~0100u;
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
        for (k = 0; k < 12u; ++k) {
            if (rnd() % 4u == 0u)
                w[FLAGWRD0_AT + k] = any_word();
            else if (rnd() % 3u == 0u)
                w[FLAGWRD0_AT + k] ^= (uint16_t)(1u << (rnd() % 15u));
        }
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A (now and then with overflow), Q. */
        k = rnd() % 4u;
        w[0] = k < 2u ? any_word() : k == 2u ? rnd() : (uint16_t)(rnd() % 2u ? 040000u : 0137777u);
        if (rnd() % 2u) w[2] = rnd() % 2u ? return_word() : any_word();
        if (rnd() % 2u) w[1] = any_word();
        if (rnd() % 2u) w[(fixloc + X1_AT + rnd() % 4u) & 03777u] = any_word();
        k = rnd() % 8u;
        w[NEWJOB_AT] = k < 5u    ? (uint16_t)(rnd() % 8u * 12u)
                       : k == 5u ? 077777u /* -0 */
                                 : (uint16_t)(rnd() & 077777u);
        w[OVFIND_AT] = rnd() % 4u ? 0u : rnd() % 2u ? 1u : 077776u;
        /* Any other word, not the interpreter's own nor FIXLOC. */
        if (rnd() % 8u == 0u) {
            k = 010u + rnd() % 03770u;
            if ((k < 0154u || k > 0166u) && k != FIXLOC_AT && k != EDOP_AT) w[k] = any_word();
        }
        /* The file's own words: DAPBOOLS's two bits, the CDUs, NDXCDUW,
         * the flags, -DELGMB, the returns, TEM3 and CDUSPOT. */
        if (rnd() % 2u) w[DAPBOOLS_AT] ^= CSMDOCKD;
        if (rnd() % 2u) w[DAPBOOLS_AT] ^= XOVINHIB;
        for (k = 0; k < 3u; ++k) {
            if (rnd() % 2u) w[CDUX_AT + k] = value_word();
            if (rnd() % 2u) w[CDUXD_AT + k] = value_word();
            if (rnd() % 2u) w[MDELGMB_AT + k] = rnd() % 2u ? edge_or_any() : value_word();
        }
        k = rnd() % 8u;
        w[NDXCDUW_AT] = k < 6u ? (uint16_t)(k % 2u) : k == 6u ? (uint16_t)(rnd() % 4u) : any_word();
        w[FLPAUTNO_AT] = flag_word(XOVINHIB);
        w[FLAGOODW_AT] = flag_word(XOVINHIB);
        /* TEM2: a return (FLTRSUB's), or CDUWXFR's count (2 to 0). */
        k = rnd() % 4u;
        if (k < 2u) w[TEM2_AT] = k == 0u ? return_word() : (uint16_t)(rnd() % 3u);
        if (rnd() % 2u) w[TEM2_AT + 1u] = edge_or_any();
        if (rnd() % 2u) w[TEM2_AT + 2u] = return_word();
        for (k = 0; k < 6u; ++k)
            if (rnd() % 2u) w[CDUSPOT_AT + k] = value_word();
        /* Now and then a zero command, in MPAC or UNWC/2 (which NORMUNIT
         * refuses with alarm 1302). */
        if (rnd() % 16u == 0u)
            for (k = 0; k < 6u; ++k) w[MPAC_AT + k] = 0u;
        if (rnd() % 16u == 0u)
            for (k = 0; k < 6u; ++k) w[UNWC_AT + k] = 0u;
        /* Channel 30's PNGCS bit (10) and channel 31's auto bit (14). */
        if (rnd() % 2u) io->in.channel30 ^= 01000u;
        if (rnd() % 2u) io->in.channel31 ^= 020000u;
        if (rnd() % 16u == 0u) io->io.superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 32u == 0u) w[INTBIT15_AT] = any_word();
        if (rnd() % 64u == 0u)
            w[3] = any_word();
        else if (rnd() % 8u == 0u)
            w[3] = (uint16_t)(rnd() % 8u << 8);
        eb = (uint16_t)(w[3] >> 8);
    }
    /* Now and then a work area the routines do not take. */
    if (n % 4u != 0u && rnd() % 64u == 0u) {
        static const uint16_t near[] = {0400u, 0402u, 0430u, 0662u, 0735u};
        k = rnd() % 3u;
        fixloc = w[FIXLOC_AT] = k == 0u   ? (uint16_t)(MPAC_AT - 044u)
                                : k == 1u ? near[rnd() % COUNT(near)]
                                          : (uint16_t)(rnd() % 02000u);
        if (fixloc >= 0401u && fixloc <= 0661u && (fixloc - 0401u) % 054u == 0u)
            fixloc = w[FIXLOC_AT] = (uint16_t)(fixloc + 1u);
    }
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u;
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    if (n % 4u != 0u && rnd() % 8u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 2u);
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            lm_channel_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            /* Channel 13 (RESETRPT's WOR): the trap bits (12-14) on or off,
             * or any; now and then no hardware record. */
            memset(&hardware, 0, sizeof hardware);
            hardware.channel13 = rnd() % 2u ? any_word() : (uint16_t)(rnd() % 8u << 11);
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            if (rnd() % 64u) {
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                                  memcmp(&rhw, &hardware, sizeof rhw)))
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
                printf("FAIL %02o,%04o case %u: %s\n", key / 1024u, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        /* Each key's faithful routine must run in at least 1 of 16
         * states, so that a change to the states cannot quietly leave a key
         * with a handful of live ones. */
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states (fewer than 1 in "
                   "16)\n",
                   key / 1024u, z, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0;

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
    failures += unreadable(&faithful_basic, false);
    failures += unreadable(&faithful_interpretive, true);
    if (failures || !mission_count) {
        printf("findcduw-cases: %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&readable_basic_table, &faithful_basic, 0, &runs);
    failures += keys(&readable_interpretive_table, &faithful_interpretive, 1, &runs);
    printf(
        "findcduw-cases: %u words as the rope's, all %u keys readable (%u basic, %u order pairs) "
        "from %u states each, %u run by the faithful routines, %u failures\n",
        words, readable_basic_table.count + readable_interpretive_table.count,
        readable_basic_table.count, readable_interpretive_table.count, CASES, runs, failures);
    return failures != 0;
}
