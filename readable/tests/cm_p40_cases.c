/* SPDX-License-Identifier: MIT */
/* The readable P40-P47 (CM) against the faithful port's, key by key
 * (make cm-p40-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_p40_routines.c) and the
 * faithful port's (c64-port/src/cm_p40_p47.c) under the same names with
 * faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p40_p47_cm/),
 * written in the listing's notation, must be the CM's rope, word for
 * word. Then the keys, which must be the faithful tables', each running
 * readable code: the file is whole. Then the runs: each key from 4,096
 * states, each one of the port's frozen test states
 * (c64-port/tests/cm_p40_p47_vectors.h) entered at the key, most stirred
 * further: MPAC and its mode, the push-down list and its pointer, the
 * index registers, A (now and then with overflow), Q (often a return the
 * file's code runs on at), the flag words (often one bit changed), the
 * vectors and times (any erasable bank's words), NEWJOB, the overflow
 * indicator, MODREG (40 or 41), MRKRTMP, PHASE5, NVWORD1 and CNTR (each
 * sign), TVCPHASE (often -1), TEMPR60 (one of GIMDTEST's words),
 * DAPDATR1 and DAPDATR2, the channels (11, 12 and 14 in the io record, 5
 * and 31 in the hardware record), now and then any word but the
 * interpreter's own, the latches, no hardware record, or a state the
 * routines refuse (another work area, BBANK not the two banks, a word of
 * 16 bits). Each runs once on each side, as the program's trampoline
 * would call the key's routine. Where either port runs, the other must
 * too, with the same memory, io record and hardware record, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run from at least 1 in 16 of
 * the states of every readable key. One refusal is counted, not failed:
 * the TC Q of SETMINDB, SETMAXDB, SPSOFF, TVCZAP and E7SETTER to FENG,
 * the constant after CALCN85's TC SERVXT, where the faithful port fails. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_p40_p47_vectors.h"

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code P40CSM_CODE, P40PVA_CODE, P40TTOG_CODE, P40S_SV_CODE,
    P41_DSP_CODE, P40CNV85_CODE, CALCN85_CODE, P41CSM_CODE, P47CSM_CODE, CALCN83_CODE, P47BODY_CODE,
    UPDATEVG_CODE, CALL40_8_CODE, SURELY_9_CODE, STEERING_CODE, DYNDISP_CODE, S40_1_CODE,
    AGAIN_CODE, S40_2_3_CODE, S41_1_CODE, S40_8_CODE, LOTHRUST_TEST_CODE, TGOCALC_CODE, S40_81_CODE,
    S40_81_END_CODE, INCRSVG_CODE, INCRSVG_END_CODE, LOTHRUST_CODE, LOTHRUST_END_CODE, S40_9_CODE,
    ENDLAMB_CODE, COPY40_9_CODE, REDO40_9_CODE, S40_13_CODE, ULLAGE_CODE, S40_133_CODE,
    NOTADDUL_CODE;
static const struct interpretive_code *const codes[] = {
    &P40CSM_CODE,   &P40PVA_CODE,        &P40TTOG_CODE,  &P40S_SV_CODE,      &P41_DSP_CODE,
    &P40CNV85_CODE, &CALCN85_CODE,       &P41CSM_CODE,   &P47CSM_CODE,       &CALCN83_CODE,
    &P47BODY_CODE,  &UPDATEVG_CODE,      &CALL40_8_CODE, &SURELY_9_CODE,     &STEERING_CODE,
    &DYNDISP_CODE,  &S40_1_CODE,         &AGAIN_CODE,    &S40_2_3_CODE,      &S41_1_CODE,
    &S40_8_CODE,    &LOTHRUST_TEST_CODE, &TGOCALC_CODE,  &S40_81_CODE,       &S40_81_END_CODE,
    &INCRSVG_CODE,  &INCRSVG_END_CODE,   &LOTHRUST_CODE, &LOTHRUST_END_CODE, &S40_9_CODE,
    &ENDLAMB_CODE,  &COPY40_9_CODE,      &REDO40_9_CODE, &S40_13_CODE,       &ULLAGE_CODE,
    &S40_133_CODE,  &NOTADDUL_CODE,
};
#define readable_basic_table        cm_p40_p47_basic
#define readable_interpretive_table cm_p40_p47_interpretive
#define faithful_basic              faithful_cm_p40_p47_basic
#define faithful_interpretive       faithful_cm_p40_p47_interpretive
#define frozen_cases                cm_p40_p47_cases
#define FROZEN_CASES                CM_P40_P47_VECTORS
#define MISSION_IS_CM               1u
#define ROPE                        1u
#define NEWOPS_AT                   06037u
extern const agc_routine_table readable_basic_table, readable_interpretive_table;
extern const agc_routine_table faithful_basic, faithful_interpretive;
bool cm_p40_p47_readable(uint16_t key, bool order_pair);
bool cm_p40_p47_runs_on(uint16_t z);

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
#define MODREG_AT   01011u
#define MRKRTMP_AT  01445u /* (in E6) */
#define PHASE5_AT   0763u
#define TVCPHASE_AT 01644u /* (in E6) */
#define TEMPR60_AT  01146u
#define CNTR_AT     01447u /* (in E6) */
#define DAPDATR1_AT 01466u /* and DAPDATR2 (in E6) */
#define NVWORD1_AT  01145u
#define QTEMP_AT    03657u /* (in E7) */
#define QTEMP1_AT   03730u /* (in E7) */

/* SETMINDB, SETMAXDB -1, SETMAXDB, SPSOFF, TVCZAP and E7SETTER, which end
 * in TC Q. */
#define SETMINDB_KEY (024u * 1024u + 0304u)
#define SETMAXDB_KEY (024u * 1024u + 0320u)
#define SPSOFF_KEY   (024u * 1024u + 01017u)
#define TVCZAP_KEY   (024u * 1024u + 01071u)
#define E7SETTER_KEY (024u * 1024u + 01366u)

#define CASES 4096u

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

/* The file is whole: every key of a faithful table runs readable code. */
static unsigned unreadable(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!cm_p40_p47_readable(key, order_pair)) {
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

/* The physical word of an erasable address, under the state's EBANK. */
static unsigned erasable_at(const uint16_t *w, unsigned address)
{
    return address < 01400u ? address : ((w[3] >> 8) & 7u) * 0400u + address - 01400u;
}

/* A return for Q: a word of bank 24 the faithful port runs on at (the
 * returns of the file's subroutines, its keys, a constant, a word it
 * stops at), or one in erasable or
 * fixed-fixed memory. */
static uint16_t a_return(void)
{
    static const uint16_t returns[] = {
        02124u, 02134u, 02135u, 02205u, 02216u, 02263u, 02073u, 02304u, 02317u, 02320u, 02437u,
        02463u, 02410u, 02555u, 02556u, 02557u, 02717u, 02720u, 02737u, 02763u, 02767u, 02775u,
        03003u, 03010u, 02570u, 02713u, 03017u, 03071u, 02724u, 03060u, 03120u, 03366u, 03573u,
        03574u, 03765u, 03225u, 03401u, 03407u, 03452u, 03530u, 03536u, 00045u, 04662u, 01776u,
        02000u, 03777u, 02020u, 02206u, 02303u, 02305u, 02307u};
    return returns[rnd() % COUNT(returns)];
}

/* Whether a key's code may run in the NOVAC work area: not the two that
 * store a vector at 6D (S40.2,3B's STCALL, BADVCTOR's STORE), nor
 * S40.13's PDDL EMDOT, whose operand (0110) is the area's first word: the
 * push can land on it, which the ports order differently (the readable
 * one loads first, as INTERPRETER.agc does). */
static bool novac_area_ok(uint16_t key)
{
    return key != 024u * 1024u + 01724u && key != 024u * 1024u + 01751u &&
           key != 016u * 1024u + 00462u;
}

/* The frozen cases. */
static unsigned mission_cases[FROZEN_CASES], mission_count;

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &frozen_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    eb = (uint16_t)(w[3] >> 8);
    if (bank >= 040u) { /* banks 40-43: FB 30-33, SUPERBNK 0100 */
        w[4] = (uint16_t)((bank - 010u) << 10);
        io->superbank = 0100u;
    } else {
        w[4] = (uint16_t)(bank << 10);
    }
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
        if (rnd() % 2u) w[2] = rnd() % 2u ? any_word() : a_return();
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
        /* MODREG (P41/P40's test): 40 or 41, or any; MRKRTMP (TRIMONLY's
         * CCS): each sign. */
        if (rnd() % 2u) w[MODREG_AT] = rnd() % 4u ? (uint16_t)(40u + rnd() % 2u) : any_word();
        if (rnd() % 2u) w[erasable_at(w, MRKRTMP_AT)] = edge_or_any();
        /* PHASE5 (PRECHECK's CCS): each sign; TVCPHASE (SPSOFF's test):
         * often -1; channel 12. */
        if (rnd() % 2u) w[PHASE5_AT] = edge_or_any();
        if (rnd() % 2u) w[erasable_at(w, TVCPHASE_AT)] = rnd() % 2u ? 077776u : edge_or_any();
        io->channel12 = any_word();
        /* NVWORD1 (CKNVWRD1's CCS, SETCLOCK's test): each sign, or verb
         * 06 noun 85 or 40; channel 11 (STEERING's RAND). */
        if (rnd() % 2u) {
            k = rnd() % 4u;
            w[NVWORD1_AT] = k == 0u ? 01525u : k == 1u ? 01450u : edge_or_any();
        }
        if (rnd() % 2u) io->channel11 = any_word();
        /* S40.6: CNTR (RSTRTST's CCS, OUTPUT's INDEX), -0 or -1 or each
         * sign; TEMPR60 (OUTPUT's TC TEMPR60), one of GIMDTEST's words;
         * channel 14. S41.2 and DAPFIG: DAPDATR1 and DAPDATR2. */
        if (rnd() % 2u) {
            k = rnd() % 4u;
            w[erasable_at(w, CNTR_AT)] = k == 0u ? 077777u : k == 1u ? 077776u : edge_or_any();
        }
        if (rnd() % 2u) w[TEMPR60_AT] = (uint16_t)(02303u + 2u * (rnd() % 3u));
        if (rnd() % 2u) io->channel14 = any_word();
        if (rnd() % 2u) w[erasable_at(w, DAPDATR1_AT)] = any_word();
        if (rnd() % 2u) w[erasable_at(w, DAPDATR1_AT + 1u)] = any_word();
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
    /* S40.2,3B's STCALL 6D and BADVCTOR's STORE 6D in a VAC area: in the
     * NOVAC one (FIXLOC 0110), 6D is FIXLOC and OVFIND themselves, and the
     * CALL or RVQ after it indexes by a FIXLOC the store has changed, the
     * whole TS QPRET added, which neither port models (as below). */
    if (fixloc == 0110u && !novac_area_ok(key)) fixloc = w[FIXLOC_AT] = 0401u;
    /* The push-down list in the work area. Not over FIXLOC and OVFIND (a
     * NOVAC job's list, from MPAC -36D, takes them in): a PUSH there makes
     * FIXLOC a word of MPAC, which a CALL's INDEX FIXLOC adds to the
     * whole TS QPRET, an instruction neither port models. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    if (w[PUSHLOC_AT] <= OVFIND_AT && w[PUSHLOC_AT] + 6u > FIXLOC_AT)
        w[PUSHLOC_AT] = OVFIND_AT + 1u;
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* QTEMP and QTEMP1, which the interpretive code returns through by
     * GOTO, fixed CADRs (as STQ leaves them): a chain of erasable CADRs
     * that never reaches a fixed one loops in the AGC until the night
     * watchman restarts it, and the faithful port's host build stops the
     * test there. */
    if (!(w[QTEMP_AT] & 074000u)) w[QTEMP_AT] = (uint16_t)(04000u + rnd() % 074000u);
    if (!(w[QTEMP1_AT] & 074000u)) w[QTEMP1_AT] = (uint16_t)(04000u + rnd() % 074000u);
    /* The interrupt latch either way now and then. */
    if (n % 4u != 0u && rnd() % 8u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 2u);
}

/* Whether the TC Q of one of them, from a state, goes to a word of
 * bank 24 the binding does not run on at: Q overflow-corrected, an
 * address in the bank. */
static bool runs_off_code(const apollo_overlay_state *s)
{
    uint16_t q = s->words[2], z = (uint16_t)((q & 037777u) | ((q >> 1) & 040000u));

    return z >= 02000u && z <= 03777u && !cm_p40_p47_runs_on(z);
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *counted)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            /* A hardware record (ULAGEOFF writes its channel 5, S40.13
             * reads its channel 31), now and then none. */
            memset(&hardware, 0, sizeof hardware);
            hardware.channel30 = any_word();
            hardware.channel5 = any_word();
            hardware.channel31 = any_word(); /* S40.13's RXOR */
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
            if (sr != EXEC_OK && !interpretive &&
                (key == SETMINDB_KEY || key == SETMAXDB_KEY - 1u || key == SETMAXDB_KEY ||
                 key == SPSOFF_KEY || key == TVCZAP_KEY || key == E7SETTER_KEY) &&
                sf != EXEC_OUT_OF_SCOPE && !memcmp(&ra, &start, sizeof ra) &&
                !memcmp(&rio, &io, sizeof rio) && runs_off_code(&start)) {
                ++*counted; /* a TC Q to FENG */
            } else if (sr != EXEC_OK &&
                       (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
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
         * states, so that a change to the states cannot quietly leave a
         * key with a handful of live ones. */
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
    unsigned i, words = 0, failures = 0, runs = 0, counted = 0;

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
        printf("cm-p40-cases: %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&readable_basic_table, &faithful_basic, 0, &runs, &counted);
    failures += keys(&readable_interpretive_table, &faithful_interpretive, 1, &runs, &counted);
    printf("cm-p40-cases: %u words as the rope's, all %u keys readable (%u basic, %u order "
           "pairs) from %u states each, %u run by the faithful routines, %u returns to FENG "
           "refused, %u failures\n",
           words, readable_basic_table.count + readable_interpretive_table.count,
           readable_basic_table.count, readable_interpretive_table.count, CASES, runs, counted,
           failures);
    return failures != 0;
}
