/* SPDX-License-Identifier: MIT */
/* The readable R30 against the faithful port's, key by key (make r30-cases
 * ROPE_LM ROPE_CM), one mission per build.
 *
 * Built with the readable tables (src/binding/r30_routines.c) and the
 * faithful port's (c64-port/src/lm_r30.c or cm_r30.c) under the same names
 * with faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/r30/r30.c), written
 * in the listing's notation, must be the mission's rope, word for word.
 * Then the keys, which must be the faithful tables'. Then the runs: each
 * key from 4,096 states, each one of the port's frozen test states for the
 * mission (c64-port/tests/r30_vectors.h) entered at the key, most stirred
 * further: MPAC and its mode, the push-down list and its pointer, the
 * index registers (X2 often 0 or 2, the earth's or the moon's), the flag
 * words, the words the vectors are read from (any erasable bank's), NEWJOB
 * (now and then -0), V82FLAGS, OPTIONX +1, MODREG and EXTVBACT, the
 * overflow indicator, now and then any word but the interpreter's own, the
 * latches, or a state the routines refuse (another work area, an EBANK no
 * program has, BBANK not the two banks, a word of 16 bits). Each runs once
 * on each side, as the program's trampoline would call the key's routine.
 * Where either port runs, the other must too, with the same memory and io
 * record, word for word; where the readable one refuses, it must leave
 * everything as it found it; and the faithful routine must run in at least
 * 1 of 16 of every key's states. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "r30_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code V82CALL_CODE, V82GOFF1_CODE, OTHSHIP_CODE, THISSHIP_CODE,
    EARTHPAD_CODE, TICKTFF_CODE, V82GON_CODE, V82GON1_CODE, SR30_1_CODE;
#if !LUMINARY099
extern const struct interpretive_code SPLRET1_CODE, CANDEL_CODE;
#endif
static const struct interpretive_code *const codes[] = {
    &V82CALL_CODE, &V82GOFF1_CODE, &OTHSHIP_CODE, &THISSHIP_CODE, &EARTHPAD_CODE,
    &TICKTFF_CODE, &V82GON_CODE,   &V82GON1_CODE, &SR30_1_CODE,
#if !LUMINARY099
    &SPLRET1_CODE, &CANDEL_CODE,
#endif
};
#if LUMINARY099
#define readable_basic        lm_r30_basic
#define readable_interpretive lm_r30_interpretive
#define faithful_basic        faithful_lm_r30_basic
#define faithful_interpretive faithful_lm_r30_interpretive
#define ROPE                  0u
#define MISSION               "LM"
#define NEWOPS_AT             06070u
#define V82FLAGS_AT           01537u /* in E-bank 4, with TFF */
#define STORE_MPAC4_AT        03604u /* SR30.1 +33 */
typedef p66_bias_io mission_io;
#else
#define readable_basic        cm_r30_basic
#define readable_interpretive cm_r30_interpretive
#define faithful_basic        faithful_cm_r30_basic
#define faithful_interpretive faithful_cm_r30_interpretive
#define ROPE                  1u
#define MISSION               "CM"
#define NEWOPS_AT             06037u
#define V82FLAGS_AT           01742u /* in E-bank 4, with TFF */
#define STORE_MPAC4_AT        02722u /* SR30.1 +33 */
typedef cm_native_io mission_io;
#endif
extern const agc_routine_table readable_basic, readable_interpretive;
extern const agc_routine_table faithful_basic, faithful_interpretive;

/* The addresses of the words R30 uses. */
#define TIME2_AT    0024u
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
#define X1_D        046u /* X1, X2, S1 and S2: work area words 38D-41D */
#define FLAGWRD0_AT 0074u
#define MODREG_AT   01011u
#define EXTVBACT_AT 01044u
#define OPTIONX_AT  01051u

#define CASES 4096u

static uint32_t seed = 3030u;
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

/* The mission's frozen cases. */
static unsigned mission_cases[R30_VECTORS], mission_count;

/* The word of the switched erasable at address (01400-01777) under E-bank
 * eb. */
static unsigned in_ebank(uint16_t eb, unsigned address)
{
    return (eb & 7u) * 0400u + (address - 01400u);
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

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hardware, uint16_t key,
                  int interpretive, unsigned n)
{
    const irc_case *c = &r30_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hardware, 0, sizeof *hardware);
    irc_build(s, c);
#if !LUMINARY099
    if (irc_ch_on) irc_cm_io(io, hardware, 0);
#endif
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
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
        /* (Not a vector at SR30.1's STORE MPAC +4, which would store over
         * MODE, LOC and BANKSET: no program does, and the two ports' LOC
         * after it, when it holds 37777, differ.) */
        if (interpretive && w[LOC_AT] == STORE_MPAC4_AT) w[MODE_AT] = (uint16_t)(k & 1u);
        for (k = 0; k < 040u; ++k)
            if (rnd() % 2u) w[(fixloc + k) & 03777u] = value_word();
        w[TIME2_AT] = value_word();
        w[TIME2_AT + 1u] = value_word();
        /* The vectors and times (RATT, VATT, RN, VN, TFF, -TPER, TSTART82
         * ...): any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* The index registers, and now and then a flag word. */
        if (rnd() % 4u == 0u) w[(fixloc + X1_D + rnd() % 4u) & 03777u] = any_word();
        if (rnd() % 2u) w[(fixloc + X1_D + 1u) & 03777u] = rnd() % 2u ? 0u : 2u; /* X2 */
        /* V82FLAGS (TFF's E-bank, 4, or the case's), OPTIONX +1 (often 1),
         * MODREG (often 0 or 11) and EXTVBACT (bit 5 or not). */
        k = rnd() % 4u;
        w[in_ebank(k ? 4u : (uint16_t)(w[3] >> 8), V82FLAGS_AT)] =
            rnd() % 4u ? (uint16_t)(rnd() % 4u) : any_word();
        if (rnd() % 2u) w[OPTIONX_AT + 1u] = rnd() % 2u ? 1u : value_word();
        if (rnd() % 2u) w[MODREG_AT] = rnd() % 3u ? (uint16_t)(rnd() % 2u ? 0u : 11u) : any_word();
        if (rnd() % 2u) w[EXTVBACT_AT] ^= 020u;
        if (rnd() % 8u == 0u) w[FLAGWRD0_AT + rnd() % 12u] = any_word();
        k = rnd() % 8u;
        w[NEWJOB_AT] = k < 5u   ? (uint16_t)(rnd() % 8u * 12u)
                       : k < 6u ? 077777u
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
        /* Now and then an EBANK no program has, BBANK most often kept
         * with it: above 03400, or with low bits. */
        if (rnd() % 32u == 0u) {
            w[3] = rnd() % 2u ? (uint16_t)(any_word() & 077400u)
                              : (uint16_t)((w[3] & 03400u) | (rnd() & 0377u));
            if (rnd() % 4u) eb = (uint16_t)(w[3] >> 8);
        }
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
            /* A hardware record mostly, which the routines do not use (the
             * CM's basic routines want one there). */
            if (rnd() % 4u) {
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
            else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
                why = "different io record";
            else if (sf == EXEC_OK && memcmp(&fhw, &rhw, sizeof fhw))
                why = "different hardware record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %02o,%04o case %u: %s\n", key / 1024u, z, n, why);
                if (getenv("R30_TRACE")) { /* the start's registers, 0-0167 */
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
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    for (i = 0; i < R30_VECTORS; ++i)
        if (r30_cases[i].cm == ROPE) mission_cases[mission_count++] = i;
    failures += tables_differ(&readable_basic, &faithful_basic, "basic");
    failures += tables_differ(&readable_interpretive, &faithful_interpretive, "interpretive");
    if (failures || mission_count == 0u) {
        printf("r30-cases (" MISSION "): %u failures\n", failures + (mission_count == 0u));
        return 1;
    }
    failures += keys(&readable_basic, &faithful_basic, 0, &runs);
    failures += keys(&readable_interpretive, &faithful_interpretive, 1, &runs);
    printf("r30-cases (" MISSION
           "): %u words as the rope's, %u keys (%u basic, %u order pairs) from %u states each, "
           "%u run by the faithful routines, %u failures\n",
           words, readable_basic.count + readable_interpretive.count, readable_basic.count,
           readable_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
