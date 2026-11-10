/* SPDX-License-Identifier: MIT */
/* The readable RTB_OP_CODES (RT8_OP_CODES in the CM) against the faithful
 * port's, key by key (make rtb-op-codes-cases ROPE_LM ROPE_CM), built once
 * per mission.
 *
 * Built with the readable tables (src/binding/rtb_op_codes_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them).
 *
 * First, in the CM, the words: QUALITY1 and QUALITY2 (src/rtb_op_codes/
 * quality.c), written in the listing's notation, must be the original
 * program's rope, word for word. Then the keys, which must be the faithful
 * tables'. Then the runs: each key from 4,096 states, each one of the
 * port's frozen test states (c64-port/tests/rtb_op_codes_vectors.h, the
 * mission's) entered at the key, most stirred further: MPAC and MODE
 * (often an edge value: +0, -0, the largest, one past half), A, L and Q
 * (now and then an overflow, a Q under 010 or past the banks), BUF and
 * TEM2 (CDUINC's CDU and its count), MPAC +2 (now and then one that lands
 * OFFTUNIT's INDEX on the central registers), the work area (FIXLOC,
 * mostly a VAC area, its X1, mostly the complement of a place in the VAC
 * areas), the CDUs, NEWJOB, any erasable word, EBANK, the latches, FB, or
 * a state the routines refuse (BBANK not the two banks, ZERO not +0, a
 * word of 16 bits, no hardware record). An order pair's state is at
 * NEWOPS with LOC the key, FLAGWRD0 (MOONFLAG) any word, now and then
 * not at NEWOPS, or with NEWJOB negative or a work area not a VAC area.
 * Each runs once on each side, as the program's trampoline would call
 * the key's routine. Where either port runs, the other must too, with the
 * same memory and io and hardware records, word for word; where the
 * readable one refuses, it must leave everything as it found it; and the
 * faithful routine must run in at least 1 of 16 of each key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "agc_transfer.h"
#include "rtb_op_codes_vectors.h"
#include "../src/agc/names.h"

#if LUMINARY099
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION      "LM"
#define CM           0u
#define BASIC        lm_rtb_op_codes_basic
#define INTERPRETIVE lm_rtb_op_codes_interpretive
#define NEWOPS_AT    06070u
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION      "CM"
#define CM           1u
#define BASIC        cm_rtb_op_codes_basic
#define INTERPRETIVE cm_rtb_op_codes_interpretive
#define NEWOPS_AT    06037u

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code QUALITY1_CODE, QUALITY2_CODE;
#endif
extern const agc_routine_table BASIC, INTERPRETIVE;
#define FAITHFUL_(name) faithful_##name
#define FAITHFUL(name)  FAITHFUL_(name)
extern const agc_routine_table FAITHFUL(BASIC), FAITHFUL(INTERPRETIVE);

/* The interpreter's words, and the work area's (from FIXLOC). */
#define AT(name) ((unsigned)K_##name)
#define X1_OFF   0046u
#define CASES    4096u

static uint32_t seed = 2718u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
}

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A word of a number: often an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 020000u,
                                     057777u, 037777u, 040000u, 010000u, 067777u};
    return pick(4u) ? any_word() : edges[pick(sizeof edges / sizeof edges[0])];
}

/* A small signed number as a word. */
static uint16_t number(int v)
{
    return (uint16_t)(v < 0 ? 077777u - (unsigned)-v : (unsigned)v);
}

/* A 16-bit register: mostly a number sign-extended, now and then an
 * overflow. */
static uint16_t register_word(uint16_t v)
{
    v = (uint16_t)(v & 040000u ? v | 0100000u : v);
    return pick(8u) ? v : (uint16_t)(v ^ 0100000u);
}

#if !LUMINARY099
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
#endif

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

/* An order pair's state: at NEWOPS with LOC the key, EDOP +0 (as DANZIG
 * leaves it before every NEWOPS), INTBIT15 the bank's bit 15 (now and then
 * not), a VAC area as the work area, PUSHLOC in it, and for most n
 * FLAGWRD0, MPAC, MODE and the work area's words stirred. */
static void pair_state(uint16_t *w, uint16_t key, unsigned n)
{
    unsigned k, fixloc;

    w[5] = NEWOPS_AT;
    w[AT(LOC)] = (uint16_t)(02000u + key % 1024u);
    w[AT(EDOP)] = 0u;
    fixloc = w[AT(FIXLOC)] = (uint16_t)(0401u + 054u * pick(5u));
    if (n % 4u != 0u) {
        w[AT(FLAGWRD0)] = any_word();
        for (k = 0; k < 7u; ++k) w[AT(MPAC) + k] = value_word();
        k = pick(3u);
        w[AT(MODE)] = k == 0u ? 0u : k == 1u ? 1u : 077776u;
        for (k = 0; k < 053u; ++k)
            if (pick(2u)) w[fixloc + k] = value_word();
        w[fixloc + X1_OFF] = (uint16_t)pick(13u);
        w[fixloc + X1_OFF + 1u] = (uint16_t)pick(13u);
    }
    w[AT(PUSHLOC)] = (uint16_t)(fixloc + pick(040u));
    w[AT(INTBIT15)] = (uint16_t)(w[4] & 040000u);
    if (n % 4u == 0u) return;
    /* Now and then a state the routines refuse: INTBIT15 not the bank's,
     * Z not NEWOPS, NEWJOB negative, or a work area that is not a VAC area.
     * (LOC is always the key: the trampoline calls the key's routine by
     * LOC.) */
    if (pick(32u) == 0u) w[AT(INTBIT15)] = any_word();
    if (pick(32u) == 0u) w[5] = pick(2u) ? (uint16_t)(NEWOPS_AT + 1u) : (uint16_t)(rnd() & 07777u);
    if (pick(32u) == 0u) w[AT(NEWJOB)] = pick(2u) ? 077777u : (uint16_t)(any_word() | 040000u);
    if (pick(32u) == 0u) {
        fixloc = w[AT(FIXLOC)] =
            pick(2u) ? (uint16_t)(AT(MPAC) - 044u) : (uint16_t)(0402u + pick(0256u));
        w[AT(PUSHLOC)] = (uint16_t)(fixloc + pick(040u));
    }
}

/* The frozen cases. */
static unsigned mission_cases[RTB_OP_CODES_VECTORS], mission_count;

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t key,
                  bool interpretive, unsigned n)
{
    const irc_case *c = &rtb_op_codes_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb, bank = (uint16_t)(key / 1024u);
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    w[4] = (uint16_t)(bank << 10);
    w[5] = (uint16_t)(02000u + key % 1024u);
    eb = (uint16_t)(w[3] <= 03400u && !(w[3] & 0377u) ? w[3] >> 8 : 0u);
    if (n % 4u != 0u) {
        /* MPAC and MODE. */
        for (k = 0; k < 7u; ++k)
            if (pick(2u)) w[AT(MPAC) + k] = value_word();
        if (pick(8u) == 0u) w[AT(MPAC) + 2u] = number((int)pick(9u) - 37);
        if (pick(2u)) {
            k = pick(4u);
            w[AT(MODE)] = k == 0u ? 0u : k == 1u ? 1u : k == 2u ? 077776u : any_word();
        }
        /* CDUINC's CDU and its count; the CDUs. */
        if (pick(4u) == 0u)
            w[AT(BUF)] = pick(2u) ? (uint16_t)(w[AT(BUF)] + pick(8u) - 4u) : value_word();
        if (pick(2u)) w[AT(TEM2)] = value_word();
        for (k = 0; k < 3u; ++k)
            if (pick(4u) == 0u) w[AT(CDUX) + k] = value_word();
        /* The work area and its X1. */
        if (pick(8u) == 0u) w[AT(FIXLOC)] = (uint16_t)(0401u + 054u * pick(5u));
        if (pick(2u)) {
            unsigned x1 = (w[AT(FIXLOC)] + X1_OFF) & 03777u;
            w[x1] = pick(4u) ? (uint16_t)(077777u - 0400u - pick(0340u)) : value_word();
        }
        w[AT(NEWJOB)] = pick(2u) ? 0u : pick(2u) ? (uint16_t)(rnd() & 037777u) : any_word();
        /* Any erasable word. */
        for (k = 010u; k < 04000u; ++k)
            if (pick(64u) == 0u) w[k] = value_word();
        w[0] = register_word(pick(2u) ? w[0] : value_word());
        w[1] = register_word(pick(2u) ? w[1] : value_word());
        k = pick(8u);
        w[2] = k < 4u    ? w[2]
               : k < 6u  ? register_word(value_word())
               : k == 6u ? (uint16_t)pick(010u)
                         : (uint16_t)(04000u + pick(074000u));
        if (pick(16u) == 0u) io->superbank = (uint16_t)(rnd() & (pick(2u) ? 0160u : 0177u));
        if (pick(32u) == 0u) io->interrupt_enabled = (uint16_t)pick(3u);
        if (pick(32u) == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (pick(16u) == 0u) {
            w[3] = pick(4u) ? (uint16_t)(pick(8u) << 8) : any_word();
            if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
        }
        /* Now and then a work area the routines do not take. */
        if (pick(64u) == 0u) w[AT(FIXLOC)] = pick(2u) ? (uint16_t)pick(02000u) : any_word();
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, ZERO
     * not +0, or a word of 16 bits. */
    if (n % 4u != 0u && pick(64u) == 0u) w[6] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[7] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[3u + pick(03775u)] |= 0100000u;
    if (interpretive) {
        pair_state(w, key, n);
        w[AT(BANKSET)] = pick(32u) ? w[6] : any_word();
    }
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     bool order_pairs, unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_hardware hw, fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, &hw, key, order_pairs, n);
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hw;
            memset(&fp, 0, sizeof fp);
            rp = fp;
            if (n % 64u != 63u) { /* now and then none */
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                                  memcmp(&rhw, &hw, sizeof rhw)))
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
                printf("FAIL %s %02o,%04o case %u: %s (status %d, faithful %d)\n", MISSION,
                       key / 1024u, 02000u + key % 1024u, n, why, (int)sr, (int)sf);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL %s %02o,%04o: the faithful routine ran in %u of %u states\n", MISSION,
                   key / 1024u, 02000u + key % 1024u, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, failures = 0, runs = 0, words = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < RTB_OP_CODES_VECTORS; ++i)
        if (rtb_op_codes_cases[i].cm == CM) mission_cases[mission_count++] = i;
#if !LUMINARY099
    failures += words_differ(&QUALITY1_CODE) + words_differ(&QUALITY2_CODE);
    words = QUALITY1_CODE.count + QUALITY2_CODE.count;
#endif
    failures += tables_differ(&BASIC, &FAITHFUL(BASIC), "basic");
    failures += tables_differ(&INTERPRETIVE, &FAITHFUL(INTERPRETIVE), "interpretive");
    if (failures) {
        printf("rtb-op-codes-cases %s: %u failures\n", MISSION, failures);
        return 1;
    }
    failures += keys(&BASIC, &FAITHFUL(BASIC), false, &runs);
    failures += keys(&INTERPRETIVE, &FAITHFUL(INTERPRETIVE), true, &runs);
    printf("rtb-op-codes-cases %s: %u words as the rope's, %u keys (%u basic, %u order pairs) "
           "from %u states each, %u faithful runs, %u failures\n",
           MISSION, words, BASIC.count + INTERPRETIVE.count, BASIC.count, INTERPRETIVE.count, CASES,
           runs, failures);
    return failures != 0;
}
