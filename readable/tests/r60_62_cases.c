/* SPDX-License-Identifier: MIT */
/* The readable R60_62 against the faithful port's, key by key (make
 * r60-r62-cases ROPE_LM ROPE_CM), one mission per build.
 *
 * Built with the readable tables (src/binding/r60_62_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/r60_62/r60_62.c),
 * written in the listing's notation, must be the original program's rope,
 * word for word. Then the keys, which must be the faithful tables'. Then
 * the runs: each key from 4,096 states, each one of the port's frozen test
 * states of the mission (c64-port/tests/r60_r62_vectors.h) entered at the
 * key, most stirred further: MPAC and its mode, the push-down list and its
 * pointer, the index registers (X1 and X2 as the program sets them, S1
 * any), A (a CADR as MAKECADR gives it, a masked bit, or any), Q (a return
 * address, or any), the saved returns (MPAC +2, TBASE1 or TBASE2, TEMPR60,
 * the LM's BALLEXIT), BUF2, MODREG (often 0, P00), the CM's OPTIND, the
 * flag words (the 3-axis, priority display, track and rendezvous flags),
 * channels 30 and 31 (G+N and auto bits often as a run has them), the
 * vectors and angles (SCAXIS, POINTVSM, CPHI, MIS, ...: any erasable
 * bank's words), NEWJOB, the overflow indicator, now and then any word but
 * the interpreter's own, the latches, or a state the routines refuse
 * (another work area, BBANK not the two banks, a word of 16 bits). Each
 * runs once on each side, as the program's trampoline would call the key's
 * routine. Where either port runs, the other must too, with the same
 * memory and io record, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * from some state of every key. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "r60_r62_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code REDOMANN_CODE, REDOMANC_CODE, VECPOINT_CODE;
#if LUMINARY099
extern const struct interpretive_code BALLANGS_CODE, ROT180_CODE;
#define readable_interpretive lm_r60_r62_interpretive
#define readable_basic        lm_r60_r62_basic
#define faithful_interpretive faithful_lm_r60_r62_interpretive
#define faithful_basic        faithful_lm_r60_r62_basic
#define MISSION_IS_CM         0
#define MISSION               "LM"
typedef lm_channel_io channel_io;
#define NEWOPS_AT 06070u
/* Unswitched. */
#define TEMPR60_AT  01164u
#define TBASE_AT    01055u /* TBASE2 */
#define BALLEXIT_AT 01342u
/* MIS and KEL, as VECPOINT's AXCs set X1 and X2. */
#define MATRIX1     03246u
#define MATRIX2     02230u
#define VECQTEMP_AT 03325u
#else
#define readable_interpretive cm_r60_r62_interpretive
#define readable_basic        cm_r60_r62_basic
#define faithful_interpretive faithful_cm_r60_r62_interpretive
#define faithful_basic        faithful_cm_r60_r62_basic
#define MISSION_IS_CM         1
#define MISSION               "CM"
typedef cm_channel_io channel_io;
#define NEWOPS_AT             06037u
/* Unswitched. */
#define TEMPR60_AT            01146u
#define TBASE_AT              01053u /* TBASE1 */
#define OPTIND_AT             01303u
/* MIS and DEL, as VECPOINT's AXCs set X1 and X2. */
#define MATRIX1               03320u
#define MATRIX2               03425u
#define VECQTEMP_AT           03310u
#endif
extern const agc_routine_table readable_basic, readable_interpretive;
extern const agc_routine_table faithful_basic, faithful_interpretive;

/* The addresses of the words the routines use. */
#define BUF2_AT     0133u
#define MODREG_AT   01011u
#define X1_AT       0046u
#define S1_AT       0050u
#define EDOP_AT     0023u
#define NEWJOB_AT   0067u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define OVFIND_AT   0121u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define FLAGWRD0_AT 0074u

#define CASES 4096u

static uint32_t seed = 4720u;
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
        if (!apollo_rope_read(MISSION_IS_CM, (uint16_t)physical, &rope) || rope != code->words[i]) {
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

/* The frozen cases of the mission. */
static unsigned mission_cases[R60_R62_VECTORS], mission_count;

/* A return address (a TC's), or any word. */
static uint16_t return_word(void)
{
    return rnd() % 4u ? (uint16_t)(010u + rnd() % 07770u) : rnd() % 2u ? any_word() : rnd();
}

/* X1 or X2 as the program sets it (AXC, and TIX's steps): the complement
 * of a place in erasable memory, one of the matrices (MIS, KEL or DEL, as
 * VECPOINT sets them) or a VAC area's word, which an indexed operand adds.
 * One that takes an indexed operand into fixed memory the AGC (and the
 * readable interpreter) reads from the rope, the faithful port from
 * erasable memory. */
static uint16_t index_word(void)
{
    static const uint16_t matrices[] = {MATRIX1, MATRIX2};
    uint16_t place = rnd() % 2u ? matrices[rnd() % (sizeof matrices / sizeof matrices[0])]
                                : (uint16_t)(0400u + rnd() % 0340u);
    return (uint16_t)(077777u - place);
}

/* The address of a key: 02000-03777 in its bank, or 04000-07777 in
 * fixed-fixed memory (banks 2 and 3). */
static uint16_t key_z(uint16_t key)
{
    return (uint16_t)(key >= 04000u && key < 010000u ? key : 02000u + key % 1024u);
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, channel_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &r60_r62_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = key_z(key), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
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
        /* The flag words (FLAGWRD0, 1, 4 and 5: rendezvous, track,
         * priority display and 3-axis), often one bit changed. */
        for (k = 0; k < 12u; ++k) {
            if (rnd() % 4u == 0u)
                w[FLAGWRD0_AT + k] = any_word();
            else if (rnd() % 4u == 0u)
                w[FLAGWRD0_AT + k] ^= (uint16_t)(1u << (rnd() % 15u));
        }
        /* The vectors and angles (SCAXIS, POINTVSM, CPHI, MIS, KEL or
         * DEL, CDUSPOT, ...): any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A: a CADR (MAKECADR's), a masked bit (+0 or one bit), or any. */
        k = rnd() % 4u;
        w[0] = k == 0u      ? (uint16_t)(rnd() & 077777u)
               : k == 1u    ? (uint16_t)(rnd() % 2u ? 0u : 1u << (rnd() % 15u))
               : rnd() % 2u ? any_word()
                            : rnd();
        /* Q and the saved returns. */
        if (rnd() % 2u) w[2] = return_word();
        if (rnd() % 2u) w[MPAC_AT + 2u] = return_word();
        if (rnd() % 2u) w[TBASE_AT] = return_word();
        if (rnd() % 2u) w[TEMPR60_AT] = any_word();
        /* BUF2: a display's Q, or any; now and then one so negative that
         * CHKLINUS's -3 + BUF2 overflows. */
        k = rnd() % 8u;
        if (k < 4u)
            w[BUF2_AT] = return_word();
        else if (k == 4u)
            w[BUF2_AT] = (uint16_t)(040000u + rnd() % 3u);
        k = rnd() % 4u;
        w[MODREG_AT] = k < 2u ? 0u : k == 2u ? (uint16_t)(rnd() % 0100u) : any_word();
#if LUMINARY099
        if (rnd() % 2u) w[BALLEXIT_AT] = any_word();
#else
        if (rnd() % 2u) w[OPTIND_AT] = any_word();
#endif
        /* Channels 30 and 31: often G+N (bit 10 of 30 clear) and auto
         * (bit 14 of 31 clear; the CM's bits 13 and 14 as PRIO30 makes
         * them pass), else any. */
        k = rnd() % 4u;
        if (k == 0u)
            io->in.channel30 &= (uint16_t)~01000u;
        else if (k == 1u)
            io->in.channel30 = any_word();
        k = rnd() % 4u;
        if (k == 0u)
            io->in.channel31 &= (uint16_t)~020000u;
        else if (k == 1u)
            io->in.channel31 = (uint16_t)((io->in.channel31 & 07777u) | 030000u);
        else if (k == 2u)
            io->in.channel31 = any_word();
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
        if (rnd() % 16u == 0u) io->io.superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 3u);
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
    /* X1 and X2 (the work area's 046 and 047); S1 (050) any. */
    w[(fixloc + X1_AT) & 03777u] = index_word();
    w[(fixloc + X1_AT + 1u) & 03777u] = index_word();
    if (n % 4u != 0u && rnd() % 2u) w[(fixloc + S1_AT) & 03777u] = any_word();
    w[6] = (uint16_t)(w[4] | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* VECQTEMP, where VECPOINT's GOTO returns through, a fixed CADR (as
     * STQ leaves it): a chain of erasable CADRs that never reaches a fixed
     * one loops in the AGC until the night watchman restarts it, and the
     * faithful port's host build stops the test there. */
    if (!(w[VECQTEMP_AT] & 074000u)) w[VECQTEMP_AT] = (uint16_t)(04000u + rnd() % 074000u);
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = key_z(key);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            channel_io io, fio, rio;
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

static const struct interpretive_code *const codes[] = {
    &REDOMANN_CODE, &REDOMANC_CODE, &VECPOINT_CODE,
#if LUMINARY099
    &BALLANGS_CODE, &ROT180_CODE,
#endif
};
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < R60_R62_VECTORS; ++i)
        if (r60_r62_cases[i].cm == MISSION_IS_CM) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&readable_basic, &faithful_basic, "basic");
    failures += tables_differ(&readable_interpretive, &faithful_interpretive, "interpretive");
    if (failures) {
        printf("r60-r62-cases (%s): %u failures\n", MISSION, failures);
        return 1;
    }
    failures += keys(&readable_basic, &faithful_basic, 0, &runs);
    failures += keys(&readable_interpretive, &faithful_interpretive, 1, &runs);
    printf("r60-r62-cases (%s): %u words as the rope's, %u keys (%u basic, %u order "
           "pairs) from %u states each, %u run by the faithful routines, %u failures\n",
           MISSION, words, readable_basic.count + readable_interpretive.count, readable_basic.count,
           readable_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
