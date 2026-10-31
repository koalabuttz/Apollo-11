/* SPDX-License-Identifier: MIT */
/* The readable LEM_GEOMETRY against the faithful port's, key by key (make
 * geometry-cases ROPE_LM ROPE_CM), the LM's only.
 *
 * Built with the readable tables (src/binding/geometry_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (SVDWN2 and SVDWN1,
 * RRANGLES, RRNB, src/geometry/geometry.c), written in the listing's
 * notation, must be the original program's rope, word for word. Then the
 * keys, which must be the faithful tables'. Then the runs: each key from
 * 4,096 states, each one of the port's frozen LM test states (c64-port/
 * tests/geometry_vectors.h) entered at the key, most stirred further: MPAC
 * and its mode, the push-down list and its pointer, the index register X2
 * the state vector shifts take (0 or 2, as the program sets it), the
 * state vector words, the radar's angles (TANGNB) and mode
 * bit (RADMODES), MODEA and MODEB, the return (S2), NEWJOB, the overflow
 * indicator, now and then any word but the interpreter's own, the
 * latches, or a state the routines refuse (another work area, BBANK not
 * the two banks, a word of 16 bits). Each runs once on each side,
 * as the program's trampoline would call the key's routine. Where either
 * port runs, the other must too, with the same memory and io record, word
 * for word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run from some state of every
 * key. */
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
extern const struct interpretive_code SVDWN_CODE, RRANGLES_CODE, RRANGLES_GOTO_CODE, RRNB_CODE;
extern const agc_routine_table lm_geometry_basic, lm_geometry_interpretive;
extern const agc_routine_table faithful_lm_geometry_basic, faithful_lm_geometry_interpretive;

/* Luminary099's addresses of the words the geometry uses. */
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
#define S2_AT       0051u
#define RADMODES_AT 0110u
#define MODEA_AT    01107u
#define TANGNB_AT   03752u
#define NEWOPS_AT   06070u

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
        if (!apollo_rope_read(0, (uint16_t)physical, &rope) || rope != code->words[i]) {
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

/* The frozen LM cases. */
static unsigned lm_cases[GEOMETRY_VECTORS], lm_count;

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
static void state(apollo_overlay_state *s, p66_bias_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &geometry_cases[lm_cases[(n * 7u + rnd()) % lm_count]];
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
        /* The state vector words and the radar's. */
        for (k = 01200u; k < 01240u; ++k)
            if (rnd() % 4u == 0u) w[k] = value_word();
        for (k = 01500u; k < 01550u; ++k)
            if (rnd() % 2u) w[k] = value_word();
        for (k = 01700u; k < 01740u; ++k)
            if (rnd() % 4u == 0u) w[k] = value_word();
        w[TANGNB_AT] = any_word();
        w[TANGNB_AT + 1u] = any_word();
        w[RADMODES_AT] = rnd() % 2u ? (uint16_t)(w[RADMODES_AT] ^ 04000u) : any_word();
        for (k = 0; k < 4u; ++k) w[MODEA_AT + k] = any_word();
        w[S2_AT] = rnd() % 2u ? (uint16_t)(02000u + rnd() % 02000u) : any_word();
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
    w[6] = (uint16_t)(w[4] | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
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
            p66_bias_io io, fio, rio;
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
                    printf("\n");
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
    const struct interpretive_code *codes[] = {&SVDWN_CODE, &RRANGLES_CODE, &RRANGLES_GOTO_CODE,
                                               &RRNB_CODE};
    unsigned i, words = 0, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < GEOMETRY_VECTORS; ++i)
        if (!geometry_cases[i].cm) lm_cases[lm_count++] = i;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&lm_geometry_basic, &faithful_lm_geometry_basic, "basic");
    failures += tables_differ(&lm_geometry_interpretive, &faithful_lm_geometry_interpretive,
                              "interpretive");
    if (failures || lm_count == 0u) {
        printf("geometry-cases: %u failures\n", failures + (lm_count == 0u));
        return 1;
    }
    failures += keys(&lm_geometry_basic, &faithful_lm_geometry_basic, 0, &runs);
    failures += keys(&lm_geometry_interpretive, &faithful_lm_geometry_interpretive, 1, &runs);
    printf("geometry-cases: %u words as the rope's, %u keys (%u basic, %u order pairs) from %u "
           "states each, %u run by the faithful routines, %u failures\n",
           words, lm_geometry_basic.count + lm_geometry_interpretive.count, lm_geometry_basic.count,
           lm_geometry_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
