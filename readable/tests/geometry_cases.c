/* SPDX-License-Identifier: MIT */
/* The readable geometry against the faithful port's, key by key, for the
 * mission it is built for (make geometry-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/geometry_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/geometry/
 * geometry.c), written in the listing's notation, must be the original
 * program's rope, word for word. Then the keys, which must be the faithful
 * tables'. Then the runs: each key from 4,096 states, each one of the
 * port's frozen test states of the mission (c64-port/tests/
 * geometry_vectors.h) entered at the key, most stirred further: MPAC and
 * its mode, the push-down list and its pointer, the index registers (X2,
 * which the state vector shifts take, 0 or 2 as the program sets it; X1,
 * the CM's mark VAC area), the state vector words, the LM's radar angles
 * (TANGNB), mode bit (RADMODES), MODEA and MODEB and return (S2), the CM's
 * star and navigation base vectors and matrices, sextant angles, earth
 * rotator vectors and time, NEWJOB, the overflow indicator, now and then
 * any word but the interpreter's own, the latches, or a state the routines
 * refuse (another work area, BBANK not the two banks, a word of 16 bits).
 * Each runs once on each side, as the program's trampoline would call the
 * key's routine. Where either port runs, the other must too, with the same
 * memory and io record, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * from some state of every key. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rope_image.h"
#include "interp_case.h"
#include "geometry_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
#if LUMINARY099
extern const struct interpretive_code SVDWN_CODE, RRANGLES_CODE, RRANGLES_GOTO_CODE, RRNB_CODE;
static const struct interpretive_code *const codes[] = {&SVDWN_CODE, &RRANGLES_CODE,
                                                        &RRANGLES_GOTO_CODE, &RRNB_CODE};
extern const agc_routine_table lm_geometry_basic, lm_geometry_interpretive;
extern const agc_routine_table faithful_lm_geometry_basic, faithful_lm_geometry_interpretive;
#define readable_basic        lm_geometry_basic
#define readable_interpretive lm_geometry_interpretive
#define faithful_basic        faithful_lm_geometry_basic
#define faithful_interpretive faithful_lm_geometry_interpretive
#define MISSION_IS_CM         0
#define MISSION               "LM"
typedef p66_bias_io mission_io;
#else
extern const struct interpretive_code SXTNB_CODE, CALCSXA_CODE, LOG_CODE, LOG_RVQ_CODE, EARROT_CODE,
    SVDWN_CODE;
static const struct interpretive_code *const codes[] = {&SXTNB_CODE,   &CALCSXA_CODE, &LOG_CODE,
                                                        &LOG_RVQ_CODE, &EARROT_CODE,  &SVDWN_CODE};
extern const agc_routine_table cm_geometry_basic, cm_geometry_interpretive;
extern const agc_routine_table faithful_cm_geometry_basic, faithful_cm_geometry_interpretive;
#define readable_basic        cm_geometry_basic
#define readable_interpretive cm_geometry_interpretive
#define faithful_basic        faithful_cm_geometry_basic
#define faithful_interpretive faithful_cm_geometry_interpretive
#define MISSION_IS_CM         1
#define MISSION               "CM"
typedef cm_native_io mission_io;
#endif

/* The addresses of the words the geometry uses (both missions'). */
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
#if LUMINARY099
#define S2_AT       0051u
#define RADMODES_AT 0110u
#define MODEA_AT    01107u
#define TANGNB_AT   03752u
#define NEWOPS_AT   06070u
#else
#define NEWOPS_AT  06037u
/* Comanche055's: STAR, XNB (through ZNB), XNB1 (through ZNB1), SAC and
 * PAC; UNITW, and RTINIT to RT and DTEAROT. */
#define STAR_AT    02765u
#define XNB_AT     02713u
#define XNB1_AT    02554u
#define SAC_AT     02773u
#define UNITW_AT   01713u
#define RTINIT_AT  03451u
#define DTEAROT_AT 03605u
#endif

#define CASES 4096u

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

/* The frozen cases of the mission. */
static unsigned mission_cases[GEOMETRY_VECTORS], mission_count;

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

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, mission_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &geometry_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
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
    if (interpretive) {
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
        k = rnd() % 3u;
        w[MODE_AT] = k == 0u ? 0u : k == 1u ? 1u : 077776u; /* DP, TP or vector */
        for (k = 0; k < 040u; ++k)
            if (rnd() % 2u) w[(fixloc + k) & 03777u] = value_word();
        /* X2, the shifts' index: 0 or 2, as the program sets it. */
        w[(fixloc + 047u) & 03777u] = rnd() % 2u ? 0u : 2u;
        /* The state vector words. */
        for (k = 01200u; k < 01240u; ++k)
            if (rnd() % 4u == 0u) w[k] = value_word();
        for (k = 01500u; k < 01550u; ++k)
            if (rnd() % 2u) w[k] = value_word();
        for (k = 01700u; k < 01740u; ++k)
            if (rnd() % 4u == 0u) w[k] = value_word();
#if LUMINARY099
        /* The radar's. */
        w[TANGNB_AT] = any_word();
        w[TANGNB_AT + 1u] = any_word();
        w[RADMODES_AT] = rnd() % 2u ? (uint16_t)(w[RADMODES_AT] ^ 04000u) : any_word();
        for (k = 0; k < 4u; ++k) w[MODEA_AT + k] = any_word();
        w[S2_AT] = rnd() % 2u ? (uint16_t)(02000u + rnd() % 02000u) : any_word();
#else
        /* The sextant's: the star, the navigation base vectors and
         * matrices, the angles; the marks' angles (below), and the return
         * at 28D a place in fixed memory. */
        for (k = 0; k < 6u; ++k)
            if (rnd() % 2u) w[STAR_AT + k] = value_word();
        for (k = 0; k < 022u; ++k) {
            if (rnd() % 2u) w[XNB_AT + k] = value_word();
            if (rnd() % 2u) w[XNB1_AT + k] = value_word();
        }
        for (k = 0; k < 4u; ++k)
            if (rnd() % 2u) w[SAC_AT + k] = value_word();
        w[(fixloc + 034u) & 03777u] = (uint16_t)(02000u + rnd() % 02000u);
        /* The earth rotator's. */
        for (k = 0; k < 6u; ++k)
            if (rnd() % 2u) w[UNITW_AT + k] = value_word();
        for (k = 0; k < 030u; ++k)
            if (rnd() % 2u) w[RTINIT_AT + k] = value_word();
        w[DTEAROT_AT] = value_word();
        w[DTEAROT_AT + 1u] = value_word();
#endif
        k = rnd() % 8u;
        w[NEWJOB_AT] = k < 5u ? (uint16_t)(rnd() % 8u * 12u) : (uint16_t)(rnd() & 077777u);
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
     * (FAKEPRET, MPAC -36D) or none. */
    if (n % 4u != 0u && rnd() % 64u == 0u)
        fixloc = w[FIXLOC_AT] =
            rnd() % 2u ? (uint16_t)(MPAC_AT - 044u) : (uint16_t)(rnd() % 02000u);
    /* The push-down list in the work area, below X1 and X2. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
#if !LUMINARY099
    /* X1 the complement of a VAC area's address, as SXTNB's callers set it
     * ("THE COMPLEMENT OF THE BASE ADDRESS OF THE MARK VAC AREA"), SXTNB
     * reading the marks' angles at its words 3 and 5. And the mode double
     * precision at each store to SAC and PAC (23,2120, 23,2136, 23,2147 and
     * 23,2151), as the RTB 1STO2S or DLOAD before it leaves it: a vector
     * stored there (two DP words at the end of E-bank 5) would run past
     * the bank. */
    {
        uint16_t area = (uint16_t)(0401u + 054u * (rnd() % 5u));
        w[(fixloc + 046u) & 03777u] = (uint16_t)(~area & 077777u);
        if (n % 4u != 0u) {
            w[area + 3u] = any_word();
            w[area + 5u] = any_word();
        }
    }
    if (interpretive && (key == 023u * 1024u + 0120u || key == 023u * 1024u + 0136u ||
                         key == 023u * 1024u + 0147u || key == 023u * 1024u + 0151u))
        w[MODE_AT] = 0u;
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
                     int interpretive, unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
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
                if (getenv("GEO_TRACE")) { /* the start's registers, 0-0167 */
                    unsigned t;
                    for (t = 0; t < 0170u; ++t)
                        printf("%s%06o", t % 8u ? " " : "\n  start ", start.words[t]);
                    t = start.words[FIXLOC_AT];
                    printf("\n  X1 %06o X2 %06o\n", start.words[(t + 046u) & 03777u],
                           start.words[(t + 047u) & 03777u]);
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
    unsigned i, words = 0, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < GEOMETRY_VECTORS; ++i)
        if (geometry_cases[i].cm == MISSION_IS_CM) mission_cases[mission_count++] = i;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&readable_basic, &faithful_basic, "basic");
    failures += tables_differ(&readable_interpretive, &faithful_interpretive, "interpretive");
    if (failures || mission_count == 0u) {
        printf("geometry-cases " MISSION ": %u failures\n", failures + (mission_count == 0u));
        return 1;
    }
    failures += keys(&readable_basic, &faithful_basic, 0, &runs);
    failures += keys(&readable_interpretive, &faithful_interpretive, 1, &runs);
    printf("geometry-cases " MISSION ": %u words as the rope's, %u keys (%u basic, %u order "
           "pairs) from %u states each, %u run by the faithful routines, %u failures\n",
           words, readable_basic.count + readable_interpretive.count, readable_basic.count,
           readable_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
