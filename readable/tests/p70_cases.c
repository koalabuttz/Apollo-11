/* SPDX-License-Identifier: MIT */
/* The readable P70-P71 against the faithful port's, key by key (make
 * p70-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/p70_routines.c) and the
 * faithful port's (c64-port/src/lm_p70_p71_file.c) under the same names
 * with faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p70_p71/goabort.c),
 * written in the listing's notation, must be the LM's rope, word for word.
 * Then the keys, which must be the faithful tables', each running readable
 * code, and the older module's tables (lm_p70_p71_basic,
 * lm_p70_p71_interpretive) must be empty. Then the runs: each key from
 * 4,096 states, each one of the port's frozen test states
 * (c64-port/tests/lm_p70_p71_file_vectors.h) entered at the key, most
 * stirred further: MPAC and its mode, the push-down list and its pointer,
 * the index registers (X1 always: mostly 0 or -8, BOTHPOLY's), A (now
 * and then with overflow), the flag words (often one bit changed), any
 * erasable bank's words, NEWJOB, the overflow indicator, now and then any word but the interpreter's own, the
 * latches, no hardware record, or a state the routines refuse (another
 * work area, BBANK not the two banks, a word of 16 bits); and the words
 * the abort's routines test: Q (P70A's -0 and P71A's 2, a return into the
 * file, or any), the program asked for and the one running (MMNUMBER,
 * MODREG: 70, 71 or any), R10,R11's pass count PIPCTR (each sign), the
 * lamp test bit, the light flash bits, AVEGFLAG and LETABORT either way,
 * channel 30's abort bits (1 and 4), channel 11 and 14, and the interrupt
 * flag. Each runs once on each side, as the program's trampoline would
 * call the key's routine. Where either port runs, the other must too, with
 * the same memory, io record and channels, word for word; where the
 * readable one refuses, it must leave everything as it found it; and the
 * faithful routine must run from at least 1 in 16 of the states of every
 * key. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "lm_p70_p71_file_vectors.h"

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code GOABORT_CODE, P70INIT_CODE, P71RET_CODE, TGOCOMP_CODE;
static const struct interpretive_code *const codes[] = {&GOABORT_CODE, &P70INIT_CODE, &P71RET_CODE,
                                                        &TGOCOMP_CODE};
extern const agc_routine_table lm_p70_p71_file_basic, lm_p70_p71_file_interpretive;
extern const agc_routine_table faithful_lm_p70_p71_file_basic,
    faithful_lm_p70_p71_file_interpretive;
extern const agc_routine_table lm_p70_p71_basic, lm_p70_p71_interpretive;
bool p70_p71_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define X1_AT       0046u /* X1, X2, S1 and S2, in the work area */
#define EDOP_AT     0023u
#define NEWJOB_AT   0067u
#define FLAGWRD0_AT 0074u
#define FLAGWRD7_AT 0103u
#define FLAGWRD9_AT 0105u
#define FLGWRD11_AT 0107u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define OVFIND_AT   0121u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define MMNUMBER_AT 0775u
#define MODREG_AT   01011u
#define PIPCTR_AT   01056u
#define IMODES33_AT 01303u
#define AVEGFBIT    020u  /* AVEGFLAG's bit of FLAGWRD7 */
#define LETABBIT    0400u /* LETABORT's bit of FLAGWRD9 */
#define NEWOPS_AT   06070u

#define CASES 4096u

static uint32_t seed = 7071u;
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
        if (!apollo_rope_read(0u, (uint16_t)physical, &rope) || rope != code->words[i]) {
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
        if (!p70_p71_readable(key, order_pair)) {
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

/* A program number: 70, 71, or any. */
static uint16_t program_number(void)
{
    unsigned k = rnd() % 4u;
    return k == 0u ? 70u : k == 1u ? 71u : k == 2u ? (uint16_t)(60u + rnd() % 16u) : any_word();
}

/* The frozen cases. */
static unsigned mission_cases[LM_P70_P71_FILE_VECTORS], mission_count;

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, lm_channel_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &lm_p70_p71_file_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
    /* Bank 32 wants the superbank bit clear. */
    if (rnd() % 4u) io->io.superbank &= (uint16_t)~0100u;
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
        /* The flag words, often one bit changed. */
        for (k = 0; k < 12u; ++k) {
            if (rnd() % 4u == 0u)
                w[FLAGWRD0_AT + k] = any_word();
            else if (rnd() % 3u == 0u)
                w[FLAGWRD0_AT + k] ^= (uint16_t)(1u << (rnd() % 15u));
        }
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A (now and then with overflow). */
        k = rnd() % 4u;
        w[0] = k < 2u ? any_word() : k == 2u ? rnd() : (uint16_t)(rnd() % 2u ? 040000u : 0137777u);
        /* The index registers but X1 (below). */
        if (rnd() % 2u) w[(fixloc + X1_AT + 1u + rnd() % 3u) & 03777u] = any_word();
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
        /* Q: P70A's -0 or P71A's 2 (ABRTJASK's index), a return into the
         * file (P70A and P71A after LEGAL?, UPTHROT's after THROTUP),
         * small, or any. */
        k = rnd() % 8u;
        if (k < 2u) {
            w[2] = k == 0u ? 077777u : 2u;
        } else if (k < 5u) {
            static const uint16_t returns[] = {02073u, 02076u, 03674u, 03700u, 03746u, 02110u};
            w[2] = returns[rnd() % COUNT(returns)];
        } else if (k < 6u) {
            w[2] = (uint16_t)(rnd() % 4u ? rnd() % 4u : 077777u - rnd() % 4u);
        } else if (k < 7u) {
            w[2] = any_word();
        }
        if (rnd() % 32u == 0u) w[2] |= 0100000u; /* (Q may hold 16 bits) */
        /* The program asked for and the one running. */
        if (rnd() % 2u) w[MMNUMBER_AT] = program_number();
        if (rnd() % 2u) w[MODREG_AT] = program_number();
        /* R10,R11's pass count, the lamp test, the lights' flash bits. */
        if (rnd() % 2u) w[PIPCTR_AT] = rnd() % 2u ? (uint16_t)(rnd() % 8u) : edge_or_any();
        if (rnd() % 2u) w[IMODES33_AT] ^= 1u;
        if (rnd() % 2u) w[FLGWRD11_AT] ^= (uint16_t)(1u + rnd() % 3u);
        /* AVEGFLAG and LETABORT either way. */
        if (rnd() % 2u) w[FLAGWRD7_AT] ^= AVEGFBIT;
        if (rnd() % 2u) w[FLAGWRD9_AT] ^= LETABBIT;
        /* Channel 30's abort (bit 1) and abort stage (bit 4) buttons. */
        if (rnd() % 2u) io->in.channel30 ^= (uint16_t)(rnd() % 2u ? 1u : rnd() % 2u ? 010u : 011u);
        if (rnd() % 16u == 0u) io->in.channel30 = any_word();
        if (rnd() % 16u == 0u) io->io.superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 3u);
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
        if (fixloc >= 0401u && fixloc <= 0661u && (fixloc - 0401u) % 054u == 0u)
            fixloc = w[FIXLOC_AT] = (uint16_t)(fixloc + 1u);
    }
    /* X1, in every state (a frozen one entered at another key has any):
     * as P70INIT and INJTARG set it with AXC,1, for the DPS's
     * coefficients (0) or the APS's (-8), else a small count. (Not a word
     * that indexes ABTCOF out of its bank: past it the faithful orders read
     * other words than the rope's, which the readable interpreter does
     * not.) */
    k = rnd() % 4u;
    w[(fixloc + X1_AT) & 03777u] = k == 0u   ? 0u
                                   : k == 1u ? 077767u
                                   : k == 2u ? (uint16_t)(rnd() % 020u)
                                             : (uint16_t)(077777u - rnd() % 020u);
    /* The push-down list in the work area. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* The interrupt latch either way now and then. */
    if (n % 4u != 0u && rnd() % 8u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 2u);
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
            lm_channel_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            /* Channels 11 and 14, and the interrupt flag (CONTABRT clears
             * it); now and then no hardware record. */
            memset(&hardware, 0, sizeof hardware);
            hardware.channel11 = rnd() % 2u ? any_word() : (uint16_t)(rnd() % 4u << 12);
            hardware.channel14 = rnd() % 2u ? any_word() : 0u;
            hardware.in_interrupt = (uint16_t)(rnd() % 2u);
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
         * states, so that a change to the states cannot quietly leave a key
         * with a handful of live ones. */
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
    for (i = 0; i < LM_P70_P71_FILE_VECTORS; ++i)
        if (!lm_p70_p71_file_cases[i].cm) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&lm_p70_p71_file_basic, &faithful_lm_p70_p71_file_basic, "basic");
    failures += tables_differ(&lm_p70_p71_file_interpretive, &faithful_lm_p70_p71_file_interpretive,
                              "interpretive");
    failures += unreadable(&faithful_lm_p70_p71_file_basic, false);
    failures += unreadable(&faithful_lm_p70_p71_file_interpretive, true);
    if (lm_p70_p71_basic.count || lm_p70_p71_interpretive.count) {
        printf("FAIL: the older module's tables are not empty\n");
        ++failures;
    }
    if (failures || !mission_count) {
        printf("p70-cases: %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&lm_p70_p71_file_basic, &faithful_lm_p70_p71_file_basic, 0, &runs);
    failures +=
        keys(&lm_p70_p71_file_interpretive, &faithful_lm_p70_p71_file_interpretive, 1, &runs);
    printf("p70-cases: %u words as the rope's, all %u keys readable (%u basic, %u order pairs) "
           "from %u states each, %u run by the faithful routines, %u failures\n",
           words, lm_p70_p71_file_basic.count + lm_p70_p71_file_interpretive.count,
           lm_p70_p71_file_basic.count, lm_p70_p71_file_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
