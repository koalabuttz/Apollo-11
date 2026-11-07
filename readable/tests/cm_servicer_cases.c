/* SPDX-License-Identifier: MIT */
/* The readable SERVICER207 of the CM against the faithful port's, key by
 * key (make cm-servicer-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_servicer_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/servicer_cm/),
 * written in the listing's notation, must be the original program's rope,
 * word for word. Then the keys, which must be the faithful tables', and
 * every key must run readable code (the file is whole). Then the runs:
 * each key from 4,096 states, each one of the port's frozen test states
 * (c64-port/tests/cm_servicer_vectors.h) entered at the key, most stirred
 * further: MPAC and its mode, the push-down list
 * and its pointer, the index registers, A, Q and RUPTREG1 (often one of
 * the words a function starts at, which a TC Q or TC RUPTREG1 goes on
 * at), AVGEXIT (often one of them, in bank 37), the words the file counts
 * and tests (PIPCTR, PIPAGE, PHASE5, DELV, TEMX to TEMZ: often +1, -1, +0
 * or -0), EBANK (often E6), TIME2 and TIME1, the flag words (often one bit
 * changed), the vectors and times (any erasable bank's words), NEWJOB (+0
 * or a core set, now and then -0), the overflow indicator, now and then
 * any word but the interpreter's own, the latches, or a state the
 * routines refuse (another work area, BBANK not the two banks, a word of
 * 16 bits, no hardware record). Each runs once
 * on each side, as the program's trampoline would call the key's routine.
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
#include "cm_servicer_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code DVTOTUP_CODE, AVERAGEG_CODE, AVGEND_CODE, NORMLIZE_CODE,
    CALCGRAV_CODE, CALCRVG_CODE;
extern const agc_routine_table cm_servicer_basic, cm_servicer_interpretive;
extern const agc_routine_table faithful_cm_servicer_basic, faithful_cm_servicer_interpretive;
bool cm_servicer_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define NEWOPS_AT       06037u
#define X1_AT           0046u
#define S1_AT           0050u
#define EDOP_AT         0023u
#define NEWJOB_AT       0067u
#define FLAGWRD0_AT     0074u
#define INTBIT15_AT     0115u
#define FIXLOC_AT       0120u
#define OVFIND_AT       0121u
#define MPAC_AT         0154u
#define MODE_AT         0163u
#define LOC_AT          0164u
#define BANKSET_AT      0165u
#define PUSHLOC_AT      0166u
#define TIME2_AT        024u
#define RUPTREG1_AT     070u
#define AVGEXIT_AT      01222u
#define PREREAD1_PLUS_3 02725u
#define AVERAGEG_EXIT   03162u
/* The words the file counts and tests: PHASE5, PIPCTR, PIPAGE, DELVX to
 * DELVZ +1, TEMX to TEMZ. */
static const uint16_t counted[] = {0763u,  01227u, 01230u, 01162u, 01163u, 01164u,
                                   01165u, 01166u, 01167u, 01224u, 01225u, 01226u};
/* The words in bank 37 a function starts at (where the faithful routine
 * runs on after a computed transfer). */
static const uint16_t labels[] = {
    02701u, 02705u, 02706u, 02712u, 02716u, 02721u, 02722u, 02744u, 02745u, 02751u, 02761u, 03005u,
    03024u, 03027u, 03033u, 03037u, 03043u, 03047u, 03052u, 03067u, 03104u, 03113u, 03114u, 03121u,
    03122u, 03131u, 03133u, 03140u, 03145u, 03151u, 03153u, 03157u, 03162u, 03165u, 03171u, 03175u,
    03201u, 03206u, 03214u, 03216u, 03225u, 03227u, 03231u, 03236u, 03242u, 03253u, 03305u};

#define CASES 4096u

static uint32_t seed = 2071u;
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
        if (!apollo_rope_read(1, (uint16_t)physical, &rope) || rope != code->words[i]) {
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

/* Every key runs a readable routine. */
static unsigned keys_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!cm_servicer_readable(key, order_pair)) {
            printf("FAIL: %02o,%04o has no readable routine\n", key / 1024u % 64u,
                   02000u + key % 1024u);
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
    return rnd() % 4u ? any_word() : edges[rnd() % (sizeof edges / sizeof edges[0])];
}

/* The frozen cases. */
static unsigned mission_cases[CM_SERVICER_VECTORS], mission_count;

/* A return address: often one of the file's words, else a TC's or any. */
static uint16_t return_word(void)
{
    if (rnd() % 2u) return labels[rnd() % (sizeof labels / sizeof labels[0])];
    return rnd() % 4u ? (uint16_t)(010u + rnd() % 07770u) : rnd() % 2u ? any_word() : rnd();
}

/* X1 and X2 as the program sets them, for the file's one indexed pair,
 * ITISMOON's BDDV* -MUDT(E),1 and SLR* 0 -21D,2: X1 the complement of
 * RTX2 (LXC,1), +0 or -2 (-0 now and then), and X2 NORM's count, the
 * complement of the shifts it took (a few more or less now and then).
 * (Not a word that indexes -MUDT(E) past the end of memory, as an X1 of
 * the complement of an erasable place does: from there yaAGC, the
 * faithful port and the readable interpreter each end with another
 * quotient.) */
static uint16_t x1_word(void)
{
    static const uint16_t x1[] = {0u, 077775u, 0u, 077775u, 077777u};
    return x1[rnd() % (sizeof x1 / sizeof x1[0])];
}

static uint16_t x2_word(void)
{
    return rnd() % 8u ? (uint16_t)(077777u - rnd() % 064u) : (uint16_t)(rnd() % 010u);
}

/* A word for CCS's four ways: +1, -1, +0 or -0, or any. */
static uint16_t ccs_word(void)
{
    static const uint16_t ways[] = {1u, 077776u, 0u, 077777u};
    return rnd() % 4u ? ways[rnd() % 4u] : any_word();
}

/* The address of a key: 02000-03777 in its bank, or 04000-07777 in
 * fixed-fixed memory (banks 2 and 3). */
static uint16_t key_z(uint16_t key)
{
    unsigned bank = key / 1024u;

    return (uint16_t)(bank == 2u || bank == 3u ? 04000u + 02000u * (bank - 2u) + key % 1024u
                                               : 02000u + key % 1024u);
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &cm_servicer_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = key_z(key), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) io->superbank &= (uint16_t)~0100u; /* banks 30-37 as such */
    if (rnd() % 4u) w[3] = 03000u;                     /* E6, AOG's */
    eb = (uint16_t)(w[3] >> 8);
    w[4] = (uint16_t)(bank < 4u ? w[4] : bank << 10);
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
        k = rnd() % 4u;
        if (k == 0u)
            w[MPAC_AT] = (uint16_t)(rnd() % 2u);
        else if (k == 1u)
            w[MPAC_AT] = rnd() % 2u ? 077777u : 077776u;
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
        /* A, Q, RUPTREG1 and AVGEXIT (a 2CADR in bank 37, or any). */
        w[0] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 2u) w[2] = return_word();
        if (rnd() % 2u) w[RUPTREG1_AT] = return_word();
        if (rnd() % 2u) {
            w[AVGEXIT_AT] = return_word();
            w[AVGEXIT_AT + 1u] = rnd() % 4u ? (uint16_t)(076000u | (rnd() % 8u)) : any_word();
        }
        /* The words the file counts and tests, each CCS's ways. */
        for (k = 0; k < sizeof counted / sizeof counted[0]; ++k)
            if (rnd() % 2u) w[counted[k]] = ccs_word();
        /* TIME2 and TIME1. */
        if (rnd() % 2u) {
            w[TIME2_AT] = any_word();
            w[TIME2_AT + 1u] = any_word();
        }
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
    }
    /* Now and then a work area the routines do not take: a NOVAC job's
     * (FAKEPRET, MPAC -36D), one a word off a VAC area, or none. */
    if (n % 4u != 0u && rnd() % 64u == 0u) {
        static const uint16_t near[] = {0400u, 0402u, 0430u, 0662u, 0735u};
        k = rnd() % 3u;
        fixloc = w[FIXLOC_AT] = k == 0u   ? (uint16_t)(MPAC_AT - 044u)
                                : k == 1u ? near[rnd() % (sizeof near / sizeof near[0])]
                                          : (uint16_t)(rnd() % 02000u);
    }
    /* The push-down list in the work area. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    /* X1 and X2 (the work area's 046 and 047), each as the program sets
     * it; S1 (050) any. */
    w[(fixloc + X1_AT) & 03777u] = x1_word();
    w[(fixloc + X1_AT + 1u) & 03777u] = x2_word();
    if (n % 4u != 0u && rnd() % 2u) w[(fixloc + S1_AT) & 03777u] = any_word();
    /* 31D, where CALCRVG keeps its return (STQ 31D), a fixed CADR: an
     * erasable one may name a cycle of erasable CADRs, which the faithful
     * interpreter stops the test at. */
    w[(fixloc + 037u) & 03777u] = (uint16_t)(04000u + rnd() % 074000u);
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* Never PREREAD1 +3 in Q or RUPTREG1: there TC RUPTREG1 would go on at
     * itself forever (PREREAD1's QXCH puts Q in RUPTREG1), in the faithful
     * routine as on the AGC. */
    if ((w[2] & 077777u) == PREREAD1_PLUS_3) w[2] = 02706u;
    if (w[RUPTREG1_AT] == PREREAD1_PLUS_3) w[RUPTREG1_AT] = 02706u;
    /* Nor AVERAGEG +017 in AVGEXIT: its DXCH Z would go on at itself. */
    if ((w[AVGEXIT_AT] & 07777u) == AVERAGEG_EXIT) w[AVGEXIT_AT] = 03227u;
    /* Now and then no hardware record: see keys(). */
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = key_z(key);
        unsigned ran = 0, bad = 0, cases = CASES;

        *readable_keys += cm_servicer_readable(key, interpretive != 0);
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            memset(&hardware, 0, sizeof hardware);
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            if (n % 64u != 63u) { /* now and then none: the basic routines refuse */
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
                printf("FAIL %02o,%04o case %u: %s\n", key / 1024u % 64u, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < cases / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states\n",
                   key / 1024u % 64u, z, ran, cases);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

static const struct interpretive_code *const codes[] = {
    &DVTOTUP_CODE, &AVERAGEG_CODE, &AVGEND_CODE, &NORMLIZE_CODE, &CALCGRAV_CODE, &CALCRVG_CODE,
};
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0, readable_basic = 0, readable_pairs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_SERVICER_VECTORS; ++i)
        if (cm_servicer_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&cm_servicer_basic, &faithful_cm_servicer_basic, "basic");
    failures += tables_differ(&cm_servicer_interpretive, &faithful_cm_servicer_interpretive,
                              "interpretive");
    failures += keys_unwritten(&faithful_cm_servicer_basic, false);
    failures += keys_unwritten(&faithful_cm_servicer_interpretive, true);
    if (failures) {
        printf("cm-servicer-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_servicer_basic, &faithful_cm_servicer_basic, 0, &runs, &readable_basic);
    failures += keys(&cm_servicer_interpretive, &faithful_cm_servicer_interpretive, 1, &runs,
                     &readable_pairs);
    printf("cm-servicer-cases: %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each, %u run by the faithful routines, "
           "%u failures\n",
           words, readable_basic + readable_pairs,
           cm_servicer_basic.count + cm_servicer_interpretive.count, readable_basic, readable_pairs,
           CASES, runs, failures);
    return failures != 0;
}
