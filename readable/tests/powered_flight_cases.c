/* SPDX-License-Identifier: MIT */
/* The readable POWERED_FLIGHT_SUBROUTINES against the faithful port's, key
 * by key, for the mission it is built for (make powered-flight-cases
 * ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/powered_flight_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/powered_flight/),
 * written in the listing's notation, must be the original program's
 * rope, word for word, and so must its constants. Then the keys, which
 * must be the faithful tables', each with a readable routine. Then the
 * runs: each key from 2,048 states, each one of the port's frozen test states
 * of the mission (c64-port/tests/lm_powered_subs_vectors.h or
 * cm_powered_subs_vectors.h) entered at the key, most stirred further:
 * MPAC and its mode, the push-down list and its pointer, the work area,
 * the E-bank, the CDU angles and counters, their sines and cosines, the
 * vector in VBUF, the flagwords, NEWJOB, EDOP, the overflow indicator,
 * now and then any word but the interpreter's own, the latches, or a
 * state the routines refuse (another work area, another bank, BBANK not
 * the two banks, a word of 16 bits). The words the file's basic code
 * keeps between its steps (the angle's offset in TEM3 or ITEMP2, the
 * returns in TEM2, ITEMP1 and RTNSAVER, AX*SR*T's counts and indexes,
 * the LM's FLESHPOT's matrix address in TEM1 and the user's EBANK in
 * TEM2) hold values the program gives them. Each runs once on each side, as
 * the program's trampoline would call the key's routine. Where either
 * port runs, the other must too, with the same memory and io record,
 * word for word; where the readable one refuses, it must leave
 * everything as it found it; and the faithful routine must run from some
 * state of every key. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rope_image.h"
#include "interp_case.h"
#if LUMINARY099
#include "lm_powered_subs_vectors.h"
#define frozen_cases lm_powered_subs_cases
#define FROZEN_CASES LM_POWERED_SUBS_VECTORS
#else
#include "cm_powered_subs_vectors.h"
#define frozen_cases cm_powered_subs_cases
#define FROZEN_CASES CM_POWERED_SUBS_VECTORS
#endif

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code CDUTRIG_CODE, CDUTRIG_RVQ_CODE, CD_STAR_TR_STAR_G_CODE,
    CD_STAR_TR_STAR_G_RVQ_CODE, CDU_STAR_SMNB_CODE, TRG_STAR_SMNB_CODE, C_STAR_MM_STAR_N2_CODE,
    CDU_STAR_NBSM_CODE, TRG_STAR_NBSM_CODE, STAR_SMNB_STAR_CODE, STAR_NBSM_STAR_CODE,
    AX_STAR_SR_STAR_T_CONSTANTS;
#if LUMINARY099
extern const struct interpretive_code CALCSMSC_CODE, CALCSMSC_RVQ_CODE;
#else
extern const struct interpretive_code NBSM_CODE, SMCDURES_CODE;
#endif
static const struct interpretive_code *const codes[] = {
    &CDUTRIG_CODE,           &CDUTRIG_RVQ_CODE,
    &CD_STAR_TR_STAR_G_CODE, &CD_STAR_TR_STAR_G_RVQ_CODE,
    &CDU_STAR_SMNB_CODE,     &TRG_STAR_SMNB_CODE,
    &C_STAR_MM_STAR_N2_CODE, &CDU_STAR_NBSM_CODE,
    &TRG_STAR_NBSM_CODE,     &STAR_SMNB_STAR_CODE,
    &STAR_NBSM_STAR_CODE,    &AX_STAR_SR_STAR_T_CONSTANTS,
#if LUMINARY099
    &CALCSMSC_CODE,          &CALCSMSC_RVQ_CODE,
#else
    &NBSM_CODE,
    &SMCDURES_CODE,
#endif
};
bool powered_flight_readable(uint16_t key, bool order_pair);
#if LUMINARY099
extern const agc_routine_table lm_powered_subs_basic, lm_powered_subs_interpretive;
extern const agc_routine_table faithful_lm_powered_subs_basic,
    faithful_lm_powered_subs_interpretive;
#define readable_basic        lm_powered_subs_basic
#define readable_interpretive lm_powered_subs_interpretive
#define faithful_basic        faithful_lm_powered_subs_basic
#define faithful_interpretive faithful_lm_powered_subs_interpretive
#define MISSION_IS_CM         0
#define MISSION               "LM"
typedef p66_bias_io mission_io;
#else
extern const agc_routine_table cm_powered_subs_basic, cm_powered_subs_interpretive;
extern const agc_routine_table faithful_cm_powered_subs_basic,
    faithful_cm_powered_subs_interpretive;
#define readable_basic        cm_powered_subs_basic
#define readable_interpretive cm_powered_subs_interpretive
#define faithful_basic        faithful_cm_powered_subs_basic
#define faithful_interpretive faithful_cm_powered_subs_interpretive
#define MISSION_IS_CM         1
#define MISSION               "CM"
typedef cm_native_io mission_io;
#endif

/* The addresses of the words the routines use (both missions'). */
#define OVFIND_AT   0121u
#define EDOP_AT     0023u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define NEWJOB_AT   0067u
#define STATE_AT    0074u /* the flagwords, STATE to STATE +11 */

#if LUMINARY099
#define NEWOPS_AT 06070u
#else
#define NEWOPS_AT 06037u
#endif
/* DANZIG, and the file's places: TR*GL**P (the words after its TC
 * USPRCADRs, entered at DANZIG, are +7, +015 and +025; the returns into
 * it +010, +016 and +026), QUICTRIG, AX*SR*T, LOOP1, and the places
 * CD*TR*GS and AX*SR*T go back to in the file. */
#if LUMINARY099
#define DANZIG_AT     06061u
#define TRGLP_Z       03560u
#define QUICTRIG_Z    03615u
#define AXSRT_Z       03675u
#define LOOP1_Z       03716u
#define CDUTRIG_Z     03537u
#define CDTRG_Z       03543u
#define CDUSMNB_Z     03646u
#define TRGSMNB_Z     03651u
#define CMMN2_Z       03655u
#define CDUNBSM_Z     03661u
#define TRGNBSM_Z     03664u
#define FLESHPOT_Z    02037u
#define FLESHLOC_BANK 010u
#else
#define DANZIG_AT  06030u
#define TRGLP_Z    03520u
#define QUICTRIG_Z 03555u
#define AXSRT_Z    03650u
#define LOOP1_Z    03671u
#define CDUTRIG_Z  03477u
#define CDTRG_Z    03503u
#define CDUSMNB_Z  03621u
#define TRGSMNB_Z  03624u
#define CMMN2_Z    03630u
#define CDUNBSM_Z  03634u
#define TRGNBSM_Z  03637u
#define NBSM_Z     03606u
#endif
#define AFTER_USPRCADR(z) ((z) == TRGLP_Z + 7u || (z) == TRGLP_Z + 015u || (z) == TRGLP_Z + 025u)
#define POWFLITE_BANK     023u
/* And the LM's TCF VXV, which leaves an EXIT in EDOP too. */
#if LUMINARY099
#define EXIT_PENDING(bank, z)                                                                      \
    (((bank) == POWFLITE_BANK && AFTER_USPRCADR(z)) ||                                             \
     ((bank) == FLESHLOC_BANK && (z) == FLESHPOT_Z + 0134u))
#else
#define EXIT_PENDING(bank, z) ((bank) == POWFLITE_BANK && AFTER_USPRCADR(z))
#endif
/* The erasable words the basic code keeps between its steps (TEM2 and
 * TEM3 share DEXDEX and DEX1). */
#define ADDRWD_AT   0116u
#define VBUF_AT     0122u
#define BUF_AT      0130u
#define TEM1_AT     0141u
#define TEM2_AT     0142u
#define TEM3_AT     0143u
#define DEXDEX_AT   0142u
#define DEX1_AT     0143u
#define RTNSAVER_AT 0145u
#define ITEMP1_AT   0061u
#define ITEMP2_AT   0062u
#define SINCDU_AT   0736u
#define CDUSPOT_AT  0766u

/* The index registers and the returns, in the work area: X1, X2, S1, S2
 * and QPRET. */
#define X1_OFFSET    046u
#define X2_OFFSET    047u
#define S1_OFFSET    050u
#define S2_OFFSET    051u
#define QPRET_OFFSET 052u

#define CASES 2048u

static uint32_t seed = 271828u;
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

/* Whether a part of the readable code is at LOC under FB. */
static int code_in(uint16_t fb, uint16_t loc)
{
    unsigned i;

    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i)
        if (fb == codes[i]->bank << 10 && loc >= codes[i]->first &&
            loc < codes[i]->first + codes[i]->count)
            return 1;
    return 0;
}

/* The frozen cases of the mission. */
static unsigned mission_cases[FROZEN_CASES], mission_count;

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

/* A return: a place the file's code is called from (the word after a TC
 * in the file, or in fixed-fixed memory), or now and then a word that is
 * none (the routines refuse it). Not a random word of erasable or
 * switchable fixed memory: there TC TEM2 can come back to itself, or
 * into a chain of erasable words that never reaches fixed memory, and
 * run for ever (the faithful port stops the program there). */
static uint16_t a_return(void)
{
    static const uint16_t places[] = {CDUTRIG_Z + 2u, CDTRG_Z + 2u,   CDUSMNB_Z + 2u,
                                      TRGSMNB_Z + 2u, CDUNBSM_Z + 2u, TRGNBSM_Z + 2u,
                                      CMMN2_Z + 1u};
    unsigned k = rnd() % 16u;
    uint16_t to;

    if (k < 8u) return places[rnd() % (sizeof places / sizeof places[0])];
    if (k < 14u) return (uint16_t)(04000u + rnd() % 04000u);
    do to = rnd() % 2u ? (uint16_t)(rnd() % 010u) : any_word();
    while (to >= 010u && to < 010000u);
    return to;
}

/* At the basic code's places, the words it keeps between its steps as
 * the program leaves them: in TR*GL**P, the offset in TEM3 (4, 2 or 0)
 * and the return in TEM2; in QUICTRIG, the offset in ITEMP2 and the
 * return in ITEMP1; at AX*SR*T, +3 or -3 in A; in LOOP1, DEXDEX +-3 to 1,
 * DEX1 and DEX2 the INDEXI pair it picks, BUF the index (1, then 0,
 * now and then any word),
 * ADDRWD at the angle's sine (or, after the sine, its cosine), and the
 * return in RTNSAVER. */
static void program_values(uint16_t *w, uint16_t z)
{
    static const uint16_t indexi[] = {4u, 2u, 0u, 4u};

    if (z > TRGLP_Z && z < TRGLP_Z + 040u) {
        w[TEM3_AT] = (uint16_t)(2u * (rnd() % 3u));
        w[TEM2_AT] = a_return();
    } else if (z > QUICTRIG_Z && z < QUICTRIG_Z + 040u) {
        w[ITEMP2_AT] = (uint16_t)(2u * (rnd() % 3u));
        w[ITEMP1_AT] = a_return();
    } else if (z == AXSRT_Z) {
        w[0] = rnd() % 2u ? 3u : 077774u;
    } else if (z > LOOP1_Z && z < LOOP1_Z + 040u) {
        unsigned count = 1u + rnd() % 3u, pair;
        int negative = rnd() % 2u;

        w[DEXDEX_AT] = negative ? (uint16_t)(077777u - count) : (uint16_t)count;
        pair = negative ? count - 1u : 3u - count;
        w[DEX1_AT] = indexi[pair];
        w[DEX1_AT + 1u] = indexi[pair + 1u];
        w[BUF_AT] = rnd() % 32u ? (uint16_t)(rnd() % 2u) : any_word();
        w[ADDRWD_AT] = (uint16_t)(SINCDU_AT + w[DEX1_AT] + (z > LOOP1_Z + 5u ? 6u : 0u));
        w[RTNSAVER_AT] = a_return();
    }
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, mission_io *io, uint16_t key, int interpretive,
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
    w[4] = (uint16_t)(bank << 10);
    if (interpretive && EXIT_PENDING(bank, z)) {
        w[5] = DANZIG_AT; /* USPRCADR's EXIT pending, LOC the CADR */
        w[LOC_AT] = z;
        w[EDOP_AT] = rnd() % 8u ? 1u : (uint16_t)(rnd() % 4u);
    } else if (interpretive) {
        w[5] = NEWOPS_AT;
        w[LOC_AT] = z;
        w[INTBIT15_AT] = (uint16_t)(w[4] & 040000u);
        /* EDOP +0, as DANZIG leaves it before every NEWOPS (a store code
         * does not write it; the faithful one takes it +0) */
        w[EDOP_AT] = 0u;
    } else {
        w[5] = z;
    }
    fixloc = w[FIXLOC_AT];
    if (n % 4u != 0u) {
        fixloc = w[FIXLOC_AT] = (uint16_t)(0401u + 054u * (rnd() % 5u));
        for (k = 0; k < 7u; ++k) w[MPAC_AT + k] = value_word();
        if (rnd() % 4u == 0u) w[MPAC_AT] = 0u; /* the earth's (BHIZ) */
        k = rnd() % 3u;
        w[MODE_AT] = k == 0u ? 0u : k == 1u ? 1u : 077776u; /* DP, TP or vector */
        /* The push-down list, 0D-37D, and the index registers. */
        for (k = 0; k < 050u; ++k)
            if (rnd() % 2u) w[(fixloc + k) & 03777u] = value_word();
        /* The rest of the work area (10D to 45D): often an edge
         * value. */
        for (k = 010u; k < 046u; ++k)
            if (rnd() % 2u) w[(fixloc + k) & 03777u] = value_word();
        /* Often the E-bank one the file's words are in (E3, E4, E5 or
         * E7), and now and then any of those banks' words. */
        if (rnd() % 2u) {
            static const uint16_t banks[] = {3u, 4u, 5u, 7u};
            eb = banks[rnd() % 4u];
            w[3] = (uint16_t)(eb << 8);
        }
        for (k = 0; k < 4u; ++k)
            if (rnd() % 4u == 0u) {
                static const uint16_t banks[] = {3u, 4u, 5u, 7u};
                unsigned j, at = banks[k] * 0400u;
                for (j = 0; j < 0400u; ++j)
                    if (rnd() % 8u == 0u) w[at + j] = value_word();
            }
        /* The flagwords: a bit now and then (GLOKFAIL, the CM's CPHIFLAG
         * and the rest). */
        for (k = 0; k < 12u; ++k)
            if (rnd() % 4u == 0u) w[STATE_AT + k] ^= (uint16_t)(1u << (rnd() % 15u));
        /* The returns: places in fixed memory, as the interpreter reads
         * them (below). */
        w[(fixloc + S1_OFFSET) & 03777u] = (uint16_t)(04000u + rnd() % 034000u);
        w[(fixloc + S2_OFFSET) & 03777u] = (uint16_t)(04000u + rnd() % 034000u);
        w[(fixloc + QPRET_OFFSET) & 03777u] = (uint16_t)(04000u + rnd() % 034000u);
        k = rnd() % 8u;
        w[NEWJOB_AT] = k < 5u ? (uint16_t)(rnd() % 8u * 12u) : (uint16_t)(rnd() & 077777u);
        w[OVFIND_AT] = rnd() % 4u ? 0u : rnd() % 2u ? 1u : 077776u;
        /* EDOP: at a basic key now and then an order waiting (a move's
         * key goes on from DANZIG only with none). */
        if (!interpretive && rnd() % 4u == 0u) w[EDOP_AT] = (uint16_t)(rnd() % 0200u);
        /* Any other word, not the interpreter's own (MPAC to PUSHLOC: LOC
         * is the key, MODE one of three; EDOP, +0 at a pair's start) nor
         * FIXLOC. */
        if (rnd() % 8u == 0u) {
            k = 010u + rnd() % 03770u;
            if ((k < 0154u || k > 0166u) && k != FIXLOC_AT && k != EDOP_AT) w[k] = any_word();
        }
        if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
        /* Now and then another bank: one with none of the file's code at
         * LOC (in one with some, the state is that key's, and the program
         * gives it that key's routine). */
        if (rnd() % 32u == 0u) {
            w[4] = (uint16_t)(rnd() & 076000u);
            if (code_in(w[4], w[LOC_AT])) w[4] = (uint16_t)(bank << 10);
        }
        if (rnd() % 32u == 0u) w[INTBIT15_AT] = any_word();
        if (rnd() % 64u == 0u) w[3] = any_word();
    }
    /* Now and then a NOVAC job's work area (FAKEPRET, MPAC -36D), one
     * place past the last VAC area, or any: the routines take only a VAC
     * area. */
    if (n % 4u != 0u && rnd() % 64u == 0u) {
        k = rnd() % 3u;
        fixloc = w[FIXLOC_AT] = k == 0u   ? (uint16_t)(MPAC_AT - 044u)
                                : k == 1u ? (uint16_t)(0661u + 054u)
                                          : (uint16_t)(rnd() % 02000u);
    }
    /* X1 and X2 small, either sign. An index the program never sets can
     * put an indexed vector across the end of an E-bank window, where the
     * faithful port's machine reads other words than the AGC's (as in
     * tests/p20_p25_cases.c). */
    w[(fixloc + X1_OFFSET) & 03777u] =
        rnd() % 2u ? (uint16_t)(rnd() % 19u) : (uint16_t)(077777u - rnd() % 16u);
    w[(fixloc + X2_OFFSET) & 03777u] =
        rnd() % 2u ? (uint16_t)(rnd() % 19u) : (uint16_t)(077777u - rnd() % 16u);
    /* The push-down list in the work area, below X1 and X2. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    /* The returns are places in fixed memory. The interpreter reads one
     * in erasable memory (HIGH4 clear) as the address of a word that
     * holds the place, and follows a chain of those that never reaches
     * fixed memory for ever (the faithful port stops the program there). */
    for (k = S1_OFFSET; k <= QPRET_OFFSET; ++k) {
        unsigned at = (w[FIXLOC_AT] + k) & 03777u;
        if (!(w[at] & 074000u)) w[at] = (uint16_t)(04000u + rnd() % 034000u);
    }
    /* The angles and their sines and cosines, the vector in VBUF. */
    if (n % 4u != 0u) {
        for (k = 0; k < 6u; ++k) {
            if (rnd() % 2u) w[CDUSPOT_AT + k] = value_word();
            if (rnd() % 2u) w[VBUF_AT + k] = value_word();
        }
        for (k = 0; k < 12u; ++k)
            if (rnd() % 2u) w[SINCDU_AT + k] = value_word();
        for (k = 0; k < 4u; ++k)
            if (rnd() % 2u) w[032u + k] = value_word(); /* the CDU counters */
    }
    if (!interpretive && bank == POWFLITE_BANK) program_values(w, z);
#if LUMINARY099
    /* At FLESHPOT, an ECADR in A, of a matrix in E1 to E7 (in E0 it would
     * be the registers); now and then overflowing, which TS TEM2 skips the
     * XCH EBANK for. */
    if (!interpretive && bank == FLESHLOC_BANK && z == FLESHPOT_Z) {
        w[0] = (uint16_t)(((1u + rnd() % 7u) << 8) | rnd() % 0400u);
        if (rnd() % 8u == 0u) w[0] |= 040000u;
    }
    /* After FLESHPOT's VXV, the matrix's address in TEM1 (01400 and the
     * ECADR's low eight bits, the matrix inside the window) and the
     * user's EBANK in TEM2. */
    if (!interpretive && bank == FLESHLOC_BANK && z == FLESHPOT_Z + 0135u) {
        w[TEM1_AT] = (uint16_t)(01400u + rnd() % 0357u);
        w[TEM2_AT] = (uint16_t)((rnd() % 8u) << 8);
    }
#endif
#if COMANCHE055
    /* At NBSM's LXC,1 (NBSM +2), the angles' address in S1, an erasable
     * one, as the program gives it ("BASE ADDRESS OF THE CDU ANGLES IS IN
     * S1"), the vector inside one E-bank (across the end of one, the
     * faithful port's machine reads other words than the AGC's). */
    if (interpretive && bank == POWFLITE_BANK && z == NBSM_Z + 2u) {
        k = rnd() % 2u ? CDUSPOT_AT : 0100u + rnd() % 01600u;
        if ((k & 0377u) > 0372u) k -= 6u;
        w[(fixloc + S1_OFFSET) & 03777u] = (uint16_t)k;
    }
#endif
    w[6] = (uint16_t)(w[4] | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *tested)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        ++*tested;
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_native_program p;
            agc_hardware hardware;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            memset(&hardware, 0, sizeof hardware);
            if (n % 64u != 63u) p.hardware = &hardware; /* now and then none */
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
                if (getenv("OI_TRACE")) { /* the start's registers, 0-0167 */
                    unsigned t;
                    for (t = 0; t < 0170u; ++t)
                        printf("%s%06o", t % 8u ? " " : "\n  start ", start.words[t]);
                    t = start.words[FIXLOC_AT];
                    printf("\n  X1 %06o X2 %06o S1 %06o\n", start.words[(t + 046u) & 03777u],
                           start.words[(t + 047u) & 03777u], start.words[(t + 050u) & 03777u]);
                }
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
    unsigned i, words = 0, failures = 0, runs = 0, tested = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < FROZEN_CASES; ++i)
        if (frozen_cases[i].cm == MISSION_IS_CM) mission_cases[mission_count++] = i;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&readable_basic, &faithful_basic, "basic");
    failures += tables_differ(&readable_interpretive, &faithful_interpretive, "interpretive");
    for (i = 0; i < faithful_basic.count; ++i) {
        uint16_t key = faithful_basic.entries[i].physical;
        if (!powered_flight_readable(key, false)) {
            printf("FAIL: basic key %02o,%04o not readable\n", key / 1024u, 02000u + key % 1024u);
            ++failures;
        }
    }
    for (i = 0; i < faithful_interpretive.count; ++i) {
        uint16_t key = faithful_interpretive.entries[i].physical;
        if (!powered_flight_readable(key, true)) {
            printf("FAIL: order pair %02o,%04o not readable\n", key / 1024u, 02000u + key % 1024u);
            ++failures;
        }
    }
    if (failures || mission_count == 0u) {
        printf("powered-flight-cases " MISSION ": %u failures\n", failures + (mission_count == 0u));
        return 1;
    }
    failures += keys(&readable_basic, &faithful_basic, 0, &runs, &tested);
    failures += keys(&readable_interpretive, &faithful_interpretive, 1, &runs, &tested);
    printf(
        "powered-flight-cases " MISSION ": %u words as the rope's, %u of %u keys readable, from %u "
        "states each, %u run by the faithful routines, %u failures\n",
        words, tested, readable_basic.count + readable_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
