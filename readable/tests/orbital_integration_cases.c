/* SPDX-License-Identifier: MIT */
/* The readable ORBITAL_INTEGRATION against the faithful port's, key by
 * key, for the mission it is built for (make orbital-integration-cases
 * ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/orbital_integration_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them).
 *
 * First the words: the readable interpretive code
 * (src/orbital_integration/), written in the listing's notation, must be
 * the original program's rope, word for word. Then the keys, which must
 * be the faithful tables'. Then the runs: each key with a readable
 * routine from 2,048 states, each one of the port's frozen test states of
 * the mission (c64-port/tests/lm_orbital_integration_vectors.h and
 * cm_orbital_integration_vectors.h) entered at the key, most stirred
 * further: MPAC and its mode, the push-down list and its pointer, the
 * work area (the index registers as the program sets them, the returns in
 * S1, S2 and QPRET), the E-bank, the words of E3, E4, E5 and E7 (the state
 * vectors, the deviations, DIFEQCNT, the times), the flagwords, NEWJOB,
 * EDOP, the overflow indicator, now and then any word but the
 * interpreter's own, the latches, or a state the routines refuse (another
 * work area, another bank, BBANK not the two banks, a word of 16 bits).
 * Each runs once on each side, as the program's trampoline would call the
 * key's routine. Where either port runs, the other must too, with the
 * same memory and io record, word for word; where the readable one
 * refuses, it must leave everything as it found it; and the faithful
 * routine must run from some state of every key. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rope_image.h"
#include "interp_case.h"
#if LUMINARY099
#include "lm_orbital_integration_vectors.h"
#define frozen_cases lm_orbital_integration_cases
#define FROZEN_CASES LM_ORBITAL_INTEGRATION_VECTORS
#else
#include "cm_orbital_integration_vectors.h"
#define frozen_cases cm_orbital_integration_cases
#define FROZEN_CASES CM_ORBITAL_INTEGRATION_VECTORS
#endif

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code KEPPREP_CODE, ACCOMP_CODE;
static const struct interpretive_code *const codes[] = {
    &KEPPREP_CODE,
    &ACCOMP_CODE,
};
bool orbital_integration_readable(uint16_t key, bool order_pair);
#if LUMINARY099
extern const agc_routine_table lm_orbital_integration_basic, lm_orbital_integration_interpretive;
extern const agc_routine_table faithful_lm_orbital_integration_basic,
    faithful_lm_orbital_integration_interpretive;
#define readable_basic        lm_orbital_integration_basic
#define readable_interpretive lm_orbital_integration_interpretive
#define faithful_basic        faithful_lm_orbital_integration_basic
#define faithful_interpretive faithful_lm_orbital_integration_interpretive
#define MISSION_IS_CM         0
#define MISSION               "LM"
typedef p66_bias_io mission_io;
#else
extern const agc_routine_table cm_orbital_integration_basic, cm_orbital_integration_interpretive;
extern const agc_routine_table faithful_cm_orbital_integration_basic,
    faithful_cm_orbital_integration_interpretive;
#define readable_basic        cm_orbital_integration_basic
#define readable_interpretive cm_orbital_integration_interpretive
#define faithful_basic        faithful_cm_orbital_integration_basic
#define faithful_interpretive faithful_cm_orbital_integration_interpretive
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
/* The keys whose operand an index register picks in a table in fixed
 * memory, which the faithful port's routines read only where the program
 * points them: MUEARTH,2 (KEPPREP's DLOAD* and PDDL*, GAMCOMP's DMP*), X2
 * the central body, PBODY (0 or 2); ASCALE,1 (GAMCOMP's DLOAD*), X1 the
 * central body's entry and 4 or 8 on, for the other bodies. */
#if LUMINARY099
static const uint16_t x2_keys[] = {011u * 1024u + 02401u % 1024u, 011u * 1024u + 02426u % 1024u,
                                   011u * 1024u + 02750u % 1024u};
static const uint16_t x1_keys[] = {011u * 1024u + 02672u % 1024u};
#else
static const uint16_t x2_keys[] = {011u * 1024u + 02375u % 1024u, 011u * 1024u + 02422u % 1024u,
                                   011u * 1024u + 02736u % 1024u};
static const uint16_t x1_keys[] = {011u * 1024u + 02660u % 1024u};
#endif
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
        /* The flagwords: a bit now and then (QUITFLAG, MOONFLAG, SURFFLAG,
         * the integration's flags and the rest). */
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
    /* X1 and X2 as the program sets them: X1 the central body's MUTABLE
     * entry (-2 for the earth, -10 for the moon), the origin (PBODY: 0 or
     * 2, and 4 or 8 on for the other bodies) or a shift count, X2 the
     * origin or a shift count (small, either sign). An index the program
     * never sets can put an indexed vector across the end of an E-bank
     * window, where the faithful port's machine reads other words than the
     * AGC's (as in tests/p20_p25_cases.c). */
    k = rnd() % 4u;
    w[(fixloc + X1_OFFSET) & 03777u] = k == 0u      ? 077775u
                                       : k == 1u    ? 077765u
                                       : rnd() % 2u ? (uint16_t)(rnd() % 19u)
                                                    : (uint16_t)(077777u - rnd() % 16u);
    w[(fixloc + X2_OFFSET) & 03777u] =
        rnd() % 2u ? (uint16_t)(rnd() % 19u) : (uint16_t)(077777u - rnd() % 16u);
    for (k = 0; interpretive && k < sizeof x2_keys / sizeof x2_keys[0]; ++k)
        if (key == x2_keys[k]) w[(fixloc + X2_OFFSET) & 03777u] = rnd() % 2u ? 0u : 2u;
    for (k = 0; interpretive && k < sizeof x1_keys / sizeof x1_keys[0]; ++k)
        if (key == x1_keys[k]) w[(fixloc + X1_OFFSET) & 03777u] = (uint16_t)(rnd() % 6u * 2u);
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

        if (!orbital_integration_readable(key, interpretive)) continue;
        ++*tested;
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
                if (getenv("OI_TRACE")) { /* the start's registers, 0-0167 */
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
    if (failures || mission_count == 0u) {
        printf("orbital-integration-cases " MISSION ": %u failures\n",
               failures + (mission_count == 0u));
        return 1;
    }
    failures += keys(&readable_basic, &faithful_basic, 0, &runs, &tested);
    failures += keys(&readable_interpretive, &faithful_interpretive, 1, &runs, &tested);
    printf("orbital-integration-cases " MISSION
           ": %u words as the rope's, %u of %u keys readable, from %u "
           "states each, %u run by the faithful routines, %u failures\n",
           words, tested, readable_basic.count + readable_interpretive.count, CASES, runs,
           failures);
    return failures != 0;
}
