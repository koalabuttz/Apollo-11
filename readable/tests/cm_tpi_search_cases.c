/* SPDX-License-Identifier: MIT */
/* The readable TPI_SEARCH of the CM against the faithful port's, key by
 * key (make cm-tpi-search-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_tpi_search_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/tpi_search/),
 * written in the listing's notation, must be the original program's rope,
 * word for word. Then the keys, which must be the faithful tables', and
 * every key must run readable code (the file is whole). Then the runs:
 * each key from 4,096 states, each one of the port's frozen test states (c64-port/tests/cm_tpi_search_vectors.h)
 * entered at the key, most stirred further: the search's words (TTPI,
 * TF and its bounds, the pericenters HP, HPO and HPERMIN, the velocities
 * DELVEE and DELVEO, the angles, the scale indexes XRS), X1 (for
 * both, mostly MUTABLE's index for the Earth or the Moon), the search
 * option OPTION2 (mostly 1 or 2), the displays' words (QSAVED, mostly a
 * return; VERBNOUN; MPAC, often BIT6), the vehicles' states, the flag words (KFLAG, AVFLAG, ITSWICH, RVSW), the work area
 * (FIXLOC, mostly a VAC area) and MPAC, A, L and Q, any erasable word,
 * EBANK (mostly E7, the file's), the latches, FB, or a state the routines
 * refuse (BBANK not the two banks, ZERO not +0, a word of 16 bits). An
 * order pair's state is at NEWOPS with LOC the key (pair_state()), now
 * and then not at NEWOPS, or with NEWJOB negative or a work area not a
 * VAC area. Each runs once on each side, as the program's trampoline
 * would call the key's routine. Where either port runs, the other must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it; and the
 * faithful routine must run in at least 1 of 16 of each key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_tpi_search_vectors.h"
#include "../src/agc/names.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code S17_1_CODE, S17_2_CODE, BIS_CODE, CONCAUL_CODE, JUNCT3_CODE,
    JUNCT1_CODE, INTERP_CODE, HALFSAFE_CODE, LRGRDVO_CODE, RNGETEST_CODE, USEKAY_CODE,
    TRANSANG_CODE, P17_1_CODE, P17_1_OPTION_CODE, ALARUMS_CODE;
extern const agc_routine_table cm_tpi_search_basic, cm_tpi_search_interpretive;
extern const agc_routine_table faithful_cm_tpi_search_basic, faithful_cm_tpi_search_interpretive;
bool cm_tpi_search_readable(uint16_t key, bool order_pair);

/* The interpreter's words, and the work area's (from FIXLOC). An
 * erasable name's place in the state is its I_ value: its address, or
 * for a switched bank's word the bank's place (E7's at 03400). */
#define NEWOPS_AT   06037u
#define EDOP_AT     0023u
#define NEWJOB_AT   0067u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define X1_OFF      0046u

#define CASES 4096u

static uint32_t seed = 1717u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
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

/* A 16-bit register: mostly a number sign-extended, now and then an
 * overflow. */
static uint16_t register_word(uint16_t v)
{
    v = (uint16_t)(v & 040000u ? v | 0100000u : v);
    return pick(8u) ? v : (uint16_t)(v ^ 0100000u);
}

/* An order pair's state: at NEWOPS with LOC the key, EDOP +0 (as DANZIG
 * leaves it before every NEWOPS), INTBIT15 the bank's bit 15 (now and then
 * not), a VAC area as the work area, PUSHLOC in it, and for most n MPAC,
 * MODE and the work area's words stirred. */
static void pair_state(uint16_t *w, uint16_t key, unsigned n)
{
    unsigned k, fixloc;

    w[5] = NEWOPS_AT;
    w[LOC_AT] = (uint16_t)(02000u + key % 1024u);
    w[EDOP_AT] = 0u;
    fixloc = w[FIXLOC_AT] = (uint16_t)(0401u + 054u * pick(5u));
    if (n % 4u != 0u) {
        for (k = 0; k < 7u; ++k) w[MPAC_AT + k] = value_word();
        k = pick(3u);
        w[MODE_AT] = k == 0u ? 0u : k == 1u ? 1u : 077776u;
        for (k = 0; k < 053u; ++k)
            if (pick(2u)) w[fixloc + k] = value_word();
        w[fixloc + X1_OFF] = (uint16_t)pick(13u);
        w[fixloc + X1_OFF + 1u] = (uint16_t)pick(13u);
    }
    w[PUSHLOC_AT] = (uint16_t)(fixloc + pick(040u));
    w[INTBIT15_AT] = (uint16_t)(w[4] & 040000u);
    if (n % 4u == 0u) return;
    /* Now and then a state the routines refuse: INTBIT15 not the bank's,
     * Z not NEWOPS, NEWJOB negative, or a work area that is not a VAC area.
     * (LOC is always the key: the trampoline calls the key's routine by
     * LOC.) */
    if (pick(32u) == 0u) w[INTBIT15_AT] = any_word();
    if (pick(32u) == 0u) w[5] = pick(2u) ? (uint16_t)(NEWOPS_AT + 1u) : (uint16_t)(rnd() & 07777u);
    if (pick(32u) == 0u) w[NEWJOB_AT] = pick(2u) ? 077777u : (uint16_t)(any_word() | 040000u);
    if (pick(32u) == 0u) {
        /* (MPAC -044, a NOVAC job's, the interpreter takes; the faithful
         * order pairs do not.) */
        fixloc = w[FIXLOC_AT] =
            pick(2u) ? (uint16_t)(MPAC_AT - 044u) : (uint16_t)(0402u + pick(0256u));
        w[PUSHLOC_AT] = (uint16_t)(fixloc + pick(040u));
    }
}

/* An index into MUTABLE: mostly the Earth's (-2) or the Moon's (-10D),
 * now and then a small one. */
static uint16_t mutable_index(void)
{
    unsigned k = pick(8u);

    return k < 3u ? 077775u : k < 6u ? 077765u : (uint16_t)pick(13u);
}

/* The search's words: each name's place and its length in words. */
static const struct {
    uint16_t at, length;
} search_words[] = {
    {I_TTPI, 2u},    {I_TF, 2u},       {I_TFI, 2u},     {I_TFO, 2u},     {I_IT, 2u},
    {I_HP, 2u},      {I_HPO, 2u},      {I_HPERMIN, 2u}, {I_DELVEE, 2u},  {I_DELVEO, 2u},
    {I_THETL, 2u},   {I_THETZERO, 2u}, {I_CSTH, 2u},    {I_SNTH, 2u},    {I_DELHITE, 2u},
    {I_MAGVTPI, 2u}, {I_RACT3, 6u},    {I_VACT3, 6u},   {I_RPASS3, 6u},  {I_VPASS3, 6u},
    {I_E2, 6u},      {I_VTPRIME, 6u},  {I_VPASS4, 6u},  {I_VIPRIME, 6u}, {I_RELDELV, 2u},
};

/* The frozen cases. */
static unsigned mission_cases[CM_TPI_SEARCH_VECTORS], mission_count;

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, bool interpretive,
                  unsigned n)
{
    const irc_case *c = &cm_tpi_search_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb, bank = (uint16_t)(key / 1024u);
    unsigned i, k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    w[4] = (uint16_t)(bank << 10);
    if (pick(4u)) io->superbank &= (uint16_t)~0100u;
    w[5] = (uint16_t)(02000u + key % 1024u);
    w[3] = pick(4u) ? 03400u : (uint16_t)(pick(8u) << 8);
    eb = (uint16_t)(w[3] >> 8);
    w[FIXLOC_AT] = (uint16_t)(0401u + 054u * pick(5u));
    if (n % 4u != 0u) {
        /* The search's words and the vehicles' states. */
        for (i = 0; i < sizeof search_words / sizeof search_words[0]; ++i)
            if (pick(2u))
                for (k = 0; k < search_words[i].length; ++k)
                    w[search_words[i].at + k] = value_word();
        /* The scale indexes: X1 mostly MUTABLE's index, the Earth's -2 or
         * the Moon's -10D, as the conic routines leave it; X2 small; now
         * and then any. */
        w[I_XRS] = pick(8u) ? mutable_index() : value_word();
        w[I_XRS + 1u] = pick(8u) ? (uint16_t)pick(13u) : value_word();
        /* The flag words: KFLAG (FLAGWRD0), AVFLAG (FLAGWRD2), ITSWICH
         * and RVSW (FLAGWRD7). */
        w[K_FLAGWRD0] = any_word();
        w[K_FLAGWRD2] = any_word();
        w[K_FLAGWRD7] = any_word();
        w[NEWJOB_AT] = pick(2u) ? 0u : (uint16_t)(rnd() & 037777u);
        /* The search option, and the displays' words: QSAVED (in E bank
         * EB, as VNCOMP17 finds it) mostly a return (a TC into fixed
         * memory), VERBNOUN, and MPAC, which a recycle tests against
         * BIT6. */
        w[I_OPTION2] = pick(4u) ? (uint16_t)(1u + pick(2u)) : value_word();
        w[eb * 0400u + K_QSAVED - 01400u] =
            pick(4u) ? (uint16_t)(02000u + pick(06000u)) : value_word();
        w[eb * 0400u + K_VERBNOUN - 01400u] = value_word();
        /* Any erasable word. */
        for (k = 010u; k < 04000u; ++k)
            if (pick(64u) == 0u) w[k] = value_word();
        w[0] = register_word(value_word());
        w[1] = register_word(value_word());
        w[2] = register_word(value_word());
        if (pick(16u) == 0u) io->superbank = (uint16_t)(rnd() & (pick(2u) ? 0160u : 0177u));
        if (pick(32u) == 0u) io->interrupt_enabled = (uint16_t)pick(3u);
        if (pick(32u) == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (pick(16u) == 0u) {
            w[3] = pick(4u) ? (uint16_t)(pick(8u) << 8) : any_word();
            if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
        }
        /* Now and then a work area the routines do not take. */
        if (pick(64u) == 0u) w[FIXLOC_AT] = pick(2u) ? (uint16_t)pick(02000u) : any_word();
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, ZERO
     * not +0, or a word of 16 bits. */
    if (n % 4u != 0u && pick(64u) == 0u) w[6] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[7] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[3u + pick(03775u)] |= 0100000u;
    if (!interpretive && n % 4u != 0u)
        w[MPAC_AT] = pick(2u)   ? (uint16_t)BIT6
                     : pick(2u) ? (uint16_t)(BIT6 ^ (1u << pick(15u)))
                                : value_word();
    if (interpretive) {
        pair_state(w, key, n);
        if (pick(2u)) w[w[FIXLOC_AT] + X1_OFF] = mutable_index();
        w[BANKSET_AT] = pick(32u) ? w[6] : any_word();
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
            cm_native_io io, fio, rio;
            agc_hardware fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, order_pairs, n);
            fa = ra = start;
            fio = rio = io;
            memset(&fhw, 0, sizeof fhw);
            rhw = fhw;
            memset(&fp, 0, sizeof fp);
            rp = fp;
            fp.hardware = &fhw;
            rp.hardware = &rhw;
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
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
                printf("FAIL %02o,%04o case %u: %s (status %d, faithful %d)\n", key / 1024u,
                       02000u + key % 1024u, n, why, (int)sr, (int)sf);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states\n", key / 1024u,
                   02000u + key % 1024u, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

static const struct interpretive_code *const codes[] = {
    /* Bank 35 */
    &P17_1_CODE,
    &P17_1_OPTION_CODE,
    &ALARUMS_CODE,
    /* Bank 36 */
    &S17_1_CODE,
    &S17_2_CODE,
    &BIS_CODE,
    &CONCAUL_CODE,
    &JUNCT3_CODE,
    &JUNCT1_CODE,
    &INTERP_CODE,
    &HALFSAFE_CODE,
    &LRGRDVO_CODE,
    &RNGETEST_CODE,
    &USEKAY_CODE,
    /* Bank 07 */
    &TRANSANG_CODE,
};

int main(int argc, char **argv)
{
    unsigned i, failures = 0, runs = 0, words = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_TPI_SEARCH_VECTORS; ++i)
        if (cm_tpi_search_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&cm_tpi_search_basic, &faithful_cm_tpi_search_basic, "basic");
    failures += tables_differ(&cm_tpi_search_interpretive, &faithful_cm_tpi_search_interpretive,
                              "interpretive");
    for (i = 0; i < faithful_cm_tpi_search_basic.count; ++i) {
        uint16_t key = faithful_cm_tpi_search_basic.entries[i].physical;
        if (!cm_tpi_search_readable(key, false)) {
            printf("FAIL: basic key %02o,%04o not readable\n", key / 1024u, 02000u + key % 1024u);
            ++failures;
        }
    }
    for (i = 0; i < faithful_cm_tpi_search_interpretive.count; ++i) {
        uint16_t key = faithful_cm_tpi_search_interpretive.entries[i].physical;
        if (!cm_tpi_search_readable(key, true)) {
            printf("FAIL: order pair %02o,%04o not readable\n", key / 1024u, 02000u + key % 1024u);
            ++failures;
        }
    }
    if (failures) {
        printf("cm-tpi-search-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_tpi_search_basic, &faithful_cm_tpi_search_basic, false, &runs);
    failures +=
        keys(&cm_tpi_search_interpretive, &faithful_cm_tpi_search_interpretive, true, &runs);
    printf("cm-tpi-search-cases: %u words as the rope's, all %u keys readable (%u basic, %u order "
           "pairs) from %u states each, %u run by the faithful routines, %u failures\n",
           words, cm_tpi_search_basic.count + cm_tpi_search_interpretive.count,
           cm_tpi_search_basic.count, cm_tpi_search_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
