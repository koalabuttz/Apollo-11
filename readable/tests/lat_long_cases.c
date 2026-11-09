/* SPDX-License-Identifier: MIT */
/* The readable LATITUDE_LONGITUDE_SUBROUTINES against the faithful
 * port's, key by key, for the mission it is built for (make
 * lat-long-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/lat_long_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/lat_long/
 * lat_long.c), written in the listing's notation, must be the original
 * program's rope, word for word. Then the keys, which must be the faithful
 * tables'. Then the runs: each key from 4,096 states, each one of the
 * port's frozen test states of the mission (c64-port/tests/
 * lat_long_vectors.h) entered at the key, most stirred further: MPAC and
 * its mode, the push-down list and its pointer, the work area (the time
 * at 6D, GAMRP, COSTH, SINTH and THETA, the return in S2), the index
 * registers, the E-bank often ALPHAV's, the vector ALPHAV, LAT, LONG,
 * ALT, ALPHAM, ERADM, the return in INCORPEX, the landing site RLS and
 * the flag words that hold LUNAFLAG and ERADFLAG, NEWJOB, the overflow
 * indicator, now and then any word but the interpreter's own, the
 * latches, or a state the routines refuse (another work area, BBANK not
 * the two banks, a word of 16 bits). Each runs once on each side, as the
 * program's trampoline would call the key's routine. Where either port
 * runs, the other must too, with the same memory and io record, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run from some state of every
 * key. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rope_image.h"
#include "interp_case.h"
#include "lat_long_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code LAT_LONG_CODE;
static const struct interpretive_code *const codes[] = {&LAT_LONG_CODE};
#if LUMINARY099
extern const agc_routine_table lm_lat_long_basic, lm_lat_long_interpretive;
extern const agc_routine_table faithful_lm_lat_long_basic, faithful_lm_lat_long_interpretive;
#define readable_basic        lm_lat_long_basic
#define readable_interpretive lm_lat_long_interpretive
#define faithful_basic        faithful_lm_lat_long_basic
#define faithful_interpretive faithful_lm_lat_long_interpretive
#define MISSION_IS_CM         0
#define MISSION               "LM"
typedef p66_bias_io mission_io;
#else
extern const agc_routine_table cm_lat_long_basic, cm_lat_long_interpretive;
extern const agc_routine_table faithful_cm_lat_long_basic, faithful_cm_lat_long_interpretive;
#define readable_basic        cm_lat_long_basic
#define readable_interpretive cm_lat_long_interpretive
#define faithful_basic        faithful_cm_lat_long_basic
#define faithful_interpretive faithful_cm_lat_long_interpretive
#define MISSION_IS_CM         1
#define MISSION               "CM"
typedef cm_native_io mission_io;
#endif

/* The addresses of the words the subroutines use (both missions'). */
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
#define FLAGWRD1_AT 0075u /* ERADFLAG's */
#define FLAGWRD3_AT 0077u /* LUNAFLAG's */
#if LUMINARY099
#define NEWOPS_AT 06070u
#define LAT_AT    01120u /* LAT, LONG and ALT */
#define ALPHAV_AT 02031u
#define ALPHAM_AT 02067u
#define RLS_AT    02022u
#define ERADM_AT  03672u /* and INCORPEX */
#else
#define NEWOPS_AT 06037u
#define LAT_AT    01103u
#define ALPHAV_AT 02151u
#define ALPHAM_AT 02307u
#define RLS_AT    02025u
#define ERADM_AT  02240u
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
static unsigned mission_cases[LAT_LONG_VECTORS], mission_count;

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
    const irc_case *c = &lat_long_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
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
        /* The words the subroutines read and write, and often the E-bank
         * ALPHAV's (E4); now and then any of the switched bank's words. */
        if (rnd() % 2u) {
            eb = (uint16_t)(ALPHAV_AT >> 8);
            w[3] = (uint16_t)(eb << 8);
        }
        if (eb >= 3u && rnd() % 4u == 0u)
            for (k = 0; k < 0400u; ++k)
                if (rnd() % 8u == 0u) w[eb * 0400u + k] = value_word();
        for (k = 0; k < 6u; ++k) {
            if (rnd() % 2u) w[ALPHAV_AT + k] = value_word();
            if (rnd() % 2u) w[LAT_AT + k] = value_word();
            if (rnd() % 4u == 0u) w[RLS_AT + k] = value_word();
        }
        for (k = 0; k < 2u; ++k) {
            if (rnd() % 2u) w[ALPHAM_AT + k] = value_word();
            if (rnd() % 2u) w[ERADM_AT + k] = value_word();
        }
        /* INCORPEX, the return LAT-LONG and LALOTORV leave by, and S2,
         * SETRE's: places in fixed memory, as the interpreter reads them
         * (below). */
        w[ERADM_AT + 2u] = (uint16_t)(04000u + rnd() % 034000u);
        w[(fixloc + 051u) & 03777u] = (uint16_t)(04000u + rnd() % 034000u);
        /* LUNAFLAG and ERADFLAG, among their flag words' bits. */
        if (rnd() % 2u) w[FLAGWRD1_AT] = any_word();
        if (rnd() % 2u) w[FLAGWRD3_AT] = any_word();
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
    /* The returns are places in fixed memory. The interpreter reads one
     * in erasable memory (HIGH4 clear) as the address of a word that
     * holds the place, and follows a chain of those that never reaches
     * fixed memory for ever (the faithful port stops the program there). */
    if (!(w[ERADM_AT + 2u] & 074000u)) w[ERADM_AT + 2u] = (uint16_t)(04000u + rnd() % 034000u);
    k = (w[FIXLOC_AT] + 051u) & 03777u;
    if (!(w[k] & 074000u)) w[k] = (uint16_t)(04000u + rnd() % 034000u);
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
                if (getenv("LL_TRACE")) { /* the start's registers, 0-0167 */
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
    for (i = 0; i < LAT_LONG_VECTORS; ++i)
        if (lat_long_cases[i].cm == MISSION_IS_CM) mission_cases[mission_count++] = i;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&readable_basic, &faithful_basic, "basic");
    failures += tables_differ(&readable_interpretive, &faithful_interpretive, "interpretive");
    if (failures || mission_count == 0u) {
        printf("lat-long-cases " MISSION ": %u failures\n", failures + (mission_count == 0u));
        return 1;
    }
    failures += keys(&readable_basic, &faithful_basic, 0, &runs);
    failures += keys(&readable_interpretive, &faithful_interpretive, 1, &runs);
    printf("lat-long-cases " MISSION ": %u words as the rope's, %u keys (%u basic, %u order "
           "pairs) from %u states each, %u run by the faithful routines, %u failures\n",
           words, readable_basic.count + readable_interpretive.count, readable_basic.count,
           readable_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
