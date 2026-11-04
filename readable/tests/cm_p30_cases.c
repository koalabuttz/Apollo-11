/* SPDX-License-Identifier: MIT */
/* The readable CM P30-P37 against the faithful port's, key by key (make
 * cm-p30-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_p30_routines.c) and the
 * faithful port's (c64-port/src/cm_p30_p37.c) under the same names with
 * faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p30_cm/), written
 * in the listing's notation, must be the original program's rope, word for
 * word. Then the keys, which must be the faithful tables', and every key
 * must run readable code (the file is whole). Then the runs: each key from
 * 4,096 states, each one of the port's frozen test states
 * (c64-port/tests/cm_p30_p37_vectors.h) entered at the key, most stirred
 * further: MPAC and its mode, the push-down list and its pointer, the
 * index registers, A (now and then with overflow, which the TSs skip on),
 * Q, the returns the basic code keeps in erasable words (P30/31RT,
 * P30/RET, PHSPRDT6, SUBEXIT: a TC's address, or any word), the flag words
 * (often one bit changed: STATE +3's bit 13, which REFTEST tests, among
 * them), the vectors and times (any erasable bank's words), NEWJOB, the
 * overflow indicator, now and then any word but the interpreter's own, the
 * latches, or a state the routines refuse (another work area, BBANK not
 * the two banks, a word of 16 bits). Each runs once on each side, as the
 * program's trampoline would call the key's routine. Where either port
 * runs, the other must too, with the same memory and io record, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run from at least 1 in 16 of
 * the states of every key. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_p30_p37_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code DISPMGA_CODE, END45_CODE, P30_CODE, P31_CODE, REFTEST_CODE,
    FLASHMGA_CODE, S30_1_CODE, S31_1_CODE, DELRSPL_CODE;
static const struct interpretive_code *const codes[] = {
    &DISPMGA_CODE,  &END45_CODE, &P30_CODE,   &P31_CODE,     &REFTEST_CODE,
    &FLASHMGA_CODE, &S30_1_CODE, &S31_1_CODE, &DELRSPL_CODE,
};
extern const agc_routine_table cm_p30_p37_basic, cm_p30_p37_interpretive;
extern const agc_routine_table faithful_cm_p30_p37_basic, faithful_cm_p30_p37_interpretive;
bool cm_p30_p37_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define NEWOPS_AT   06037u
#define X1_AT       0046u /* X1, X2, S1 and S2, in the work area */
#define EDOP_AT     0023u
#define NEWJOB_AT   0067u
#define FLAGWRD0_AT 0074u
#define STATE3_AT   0077u /* STATE +3: bit 13, REFTEST's */
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define OVFIND_AT   0121u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define QPRET_AT    052u   /* the work area's QPRET */
#define PHSPRDT6_AT 01066u /* COMPTGO's return, a TC's address */
#define P30_RET_AT  01127u /* P30/RET, CNTNUP30's return */
#define P30_31RT_AT 01664u /* P30/31RT in the E bank, P30/P31's and PARAM30's return */
#define SUBEXIT_AT  02370u /* E4,1770: SUBEXIT (and RGEXIT), a return */
#define QTEMP_AT    03657u /* E7,1657: S30.1's and S31.1's return, a CADR */

#define CASES 4096u

static uint32_t seed = 3037u;
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

/* The file is whole: every key runs readable code. */
static unsigned keys_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!cm_p30_p37_readable(key, order_pair)) {
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
    return rnd() % 4u ? any_word() : edges[rnd() % (sizeof edges / sizeof edges[0])];
}

/* The frozen cases. */
static unsigned mission_cases[CM_P30_P37_VECTORS], mission_count;

/* A return address (a TC's), or any word: a TC through an erasable word
 * takes only an address 010-07777. */
static uint16_t return_word(void)
{
    return rnd() % 4u ? (uint16_t)(010u + rnd() % 07770u) : rnd() % 2u ? any_word() : rnd() % 010u;
}

/* The address of a key: 02000-03777 in its bank, or 04000-07777 in
 * fixed-fixed memory (banks 2 and 3). */
static uint16_t key_z(uint16_t key)
{
    unsigned bank = key / 1024u;

    return (uint16_t)(bank == 2u || bank == 3u ? 04000u + 02000u * (bank - 2u) + key % 1024u
                                               : 02000u + key % 1024u);
}

/* A CADR in fixed memory, as STQ leaves one: mostly the word now if it
 * is one, else any. A GOTO through an erasable word takes the CADR there. */
static uint16_t fixed_cadr(uint16_t now)
{
    return (now & 074000u) ? now : (uint16_t)(04000u + rnd() % 074000u);
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hardware, uint16_t key,
                  int interpretive, unsigned n)
{
    const irc_case *c = &cm_p30_p37_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = key_z(key), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hardware, 0, sizeof *hardware);
    irc_build(s, c);
    if (irc_ch_on) irc_cm_io(io, hardware, 0);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    /* The file's banks (31, 32 and 35) want the superbank bit clear. */
    if (rnd() % 4u) io->superbank &= (uint16_t)~0100u;
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
        k = rnd() % 3u;
        w[MODE_AT] = k == 0u ? 0u : k == 1u ? 1u : 077776u; /* DP, TP or vector */
        for (k = 0; k < 040u; ++k)
            if (rnd() % 2u) w[(fixloc + k) & 03777u] = value_word();
        /* The flag words, often one bit changed (STATE +3's bit 13 half
         * the time either way). */
        for (k = 0; k < 12u; ++k) {
            if (rnd() % 4u == 0u)
                w[FLAGWRD0_AT + k] = any_word();
            else if (rnd() % 3u == 0u)
                w[FLAGWRD0_AT + k] ^= (uint16_t)(1u << (rnd() % 15u));
        }
        if (rnd() % 2u) w[STATE3_AT] ^= 010000u;
        /* The vectors and times: any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A (now and then with overflow), Q. */
        k = rnd() % 4u;
        w[0] = k < 2u ? any_word() : k == 2u ? rnd() : (uint16_t)(rnd() % 2u ? 040000u : 0137777u);
        if (rnd() % 2u) w[2] = return_word();
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
        /* The returns the basic code keeps: a TC's address, or any word. */
        if (rnd() % 2u) w[PHSPRDT6_AT] = return_word();
        if (rnd() % 2u) w[P30_RET_AT] = return_word();
        if (rnd() % 2u) w[((eb & 7u) * 0400u + P30_31RT_AT - 01400u) & 03777u] = return_word();
        if (rnd() % 2u) w[SUBEXIT_AT] = return_word();
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
    /* In bank 32, often X1 0 to 12, the counts K1K2LOOP's loads from
     * AUGEKUGL's tables run on, and now and then -5 to 17, round their
     * ends. */
    if (bank == 032u) {
        k = rnd() % 4u;
        if (k < 2u)
            w[(fixloc + X1_AT) & 03777u] = (uint16_t)(rnd() % 13u);
        else if (k == 2u) {
            k = rnd() % 23u;
            w[(fixloc + X1_AT) & 03777u] = (uint16_t)(k < 5u ? 077777u - (k + 1u) : k - 5u);
        }
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* QTEMP and RGEXIT (SUBEXIT's word), the returns S30.1, S31.1 and
     * DISPMGA's GOTOs take, and the work area's QPRET, which RVQ takes:
     * CADRs in fixed memory as the STQs leave them. (One in erasable
     * memory could send the faithful GOTO round a cycle of CADRs, which it
     * ends by exiting.) A basic key, which TCs through SUBEXIT, takes it as
     * any return. */
    w[QTEMP_AT] = fixed_cadr(w[QTEMP_AT]);
    if (interpretive) w[SUBEXIT_AT] = fixed_cadr(w[SUBEXIT_AT]);
    w[(fixloc + QPRET_AT) & 03777u] = fixed_cadr(w[(fixloc + QPRET_AT) & 03777u]);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* The interrupt latch either way now and then. */
    if (n % 4u != 0u && rnd() % 8u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 2u);
}

/* The keys whose readable routine ran in at least one state. */
static unsigned readable_keys;

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = key_z(key);
        unsigned ran = 0, bad = 0, readable_ran = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, &hardware, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            /* A hardware record mostly, which the routines do not use. */
            if (rnd() % 4u) {
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            readable_ran += sr == EXEC_OK;
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
         * states, so that a change to the states cannot quietly leave a
         * key with a handful of live ones. */
        readable_keys += readable_ran != 0u;
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
    for (i = 0; i < CM_P30_P37_VECTORS; ++i)
        if (cm_p30_p37_cases[i].cm) mission_cases[mission_count++] = i;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&cm_p30_p37_basic, &faithful_cm_p30_p37_basic, "basic");
    failures +=
        tables_differ(&cm_p30_p37_interpretive, &faithful_cm_p30_p37_interpretive, "interpretive");
    failures += keys_unwritten(&cm_p30_p37_basic, false);
    failures += keys_unwritten(&cm_p30_p37_interpretive, true);
    if (failures || !mission_count) {
        printf("cm-p30-cases: %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&cm_p30_p37_basic, &faithful_cm_p30_p37_basic, 0, &runs);
    failures += keys(&cm_p30_p37_interpretive, &faithful_cm_p30_p37_interpretive, 1, &runs);
    if (readable_keys != cm_p30_p37_basic.count + cm_p30_p37_interpretive.count) {
        printf("FAIL: %u keys whose readable routine never ran\n",
               cm_p30_p37_basic.count + cm_p30_p37_interpretive.count - readable_keys);
        ++failures;
    }
    printf("cm-p30-cases: %u words as the rope's, %u of %u keys run by the readable routines "
           "(%u basic, %u order pairs) from %u states each, %u run by the faithful routines, %u "
           "failures\n",
           words, readable_keys, cm_p30_p37_basic.count + cm_p30_p37_interpretive.count,
           cm_p30_p37_basic.count, cm_p30_p37_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
