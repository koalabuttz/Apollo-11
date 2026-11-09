/* SPDX-License-Identifier: MIT */
/* The readable P11 of the CM against the faithful port's, key by key
 * (make cm-p11-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_p11_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p11/), written in
 * the listing's notation, must be the original program's rope, word for
 * word. Then the keys, which must be the faithful tables', and every key
 * must run readable code (the file is whole). Then the runs: each key
 * from 4,096 states, each one of the port's frozen test states
 * (c64-port/tests/cm_p11_vectors.h) entered at the key, most stirred
 * further: the restart words P11 keeps (LIFTTEMP, TLIFTOFF, PHASE5, the
 * clock), the takeover's switch SATSW, FLAGWRD6's Saturn stick bits, the
 * boost's times (RPSTART, POLYSTOP, TIME1 and TIME2 about them), the
 * attitude errors and their bias and scale (now and then large enough to
 * overflow), the roll test's MPAC and SATRLRT, the hand controller's rate
 * indexes (mostly within SATRATE's table, now and then one that lands
 * the INDEX at Z, on the central registers or out of erasable memory),
 * the work area (FIXLOC, mostly a VAC area), channels 12, 30 and 31, A,
 * L and Q, any erasable word, EBANK (mostly E6, the file's), the latches,
 * FB, or a state the routines refuse (BBANK not the two banks, ZERO not
 * +0, a word of 16 bits, no hardware record). An order pair's state is
 * at NEWOPS with LOC the key (pair_state()), now and then not at NEWOPS,
 * or with NEWJOB negative or a work area not a VAC area. Each runs once
 * on each side, as the program's trampoline would call the key's
 * routine. Where either
 * port runs, the other must too, with the same memory and io and
 * hardware records, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * in at least 1 of 16 of each key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_p11_vectors.h"
#include "../src/agc/names.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code P11_CODE, MATRXJOB_CODE, VHHDOT_CODE, GETDOWN_CODE,
    ATTDISP_CODE, ATTDISP2_CODE, STEERSAT_CODE, S11_1_CODE, NOPOLY_CODE, ATTDISP1_CODE;
extern const agc_routine_table cm_p11_basic, cm_p11_interpretive;
extern const agc_routine_table faithful_cm_p11_basic, faithful_cm_p11_interpretive;
bool cm_p11_readable(uint16_t key, bool order_pair);

/* A word's place in the state: E6's words (BODY3's bank, the file's) at
 * 03000 + (the address - 01400), E3's (TEPHEM's) and unswitched ones at
 * their address. */
#define AT6(name) ((unsigned)K_##name + 01400u)
#define AT(name)  ((unsigned)K_##name)

/* The interpreter's words, and the work area's (from FIXLOC). */
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

static uint32_t seed = 1101u;
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

/* A sign CCS branches on: often +0, -0, 1 or -1. */
static uint16_t sign_word(void)
{
    unsigned k = pick(8u);

    return k < 2u ? 0u : k == 2u ? 077777u : k == 3u ? 1u : k == 4u ? 077776u : value_word();
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

/* A rate index as STICKCHK leaves it (0, 1 or 2), now and then one that
 * lands the INDEX at Z, on A, L or Q, or out of erasable memory, or any. */
static uint16_t rate_index(void)
{
    unsigned k = pick(16u);

    return k < 12u    ? (uint16_t)pick(3u)
           : k == 12u ? number(5 - (int)K_SATRATE)
           : k == 13u ? number((int)pick(3u) - (int)K_SATRATE)
           : k == 14u ? number(02000 - (int)K_SATRATE + (int)pick(4u))
                      : value_word();
}

/* A time about another: often a little before or after it. */
static uint16_t near(uint16_t t)
{
    return pick(2u) ? (uint16_t)((t + 077770u + pick(020u)) % 077777u) : value_word();
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

/* The frozen cases. */
static unsigned mission_cases[CM_P11_VECTORS], mission_count;

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hw, uint16_t key,
                  bool interpretive, unsigned n)
{
    const irc_case *c = &cm_p11_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb, bank = (uint16_t)(key / 1024u);
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    /* The key's bank: bank 43 is FB 33 with the superbank bit. */
    if (bank >= 040u) {
        w[4] = (uint16_t)((bank - 010u) << 10);
        io->superbank = 0100u; /* (the superbank bit) */
    } else {
        w[4] = (uint16_t)(bank << 10);
        if (pick(4u)) io->superbank &= (uint16_t)~0100u;
    }
    w[5] = (uint16_t)(02000u + key % 1024u);
    w[3] = pick(4u) ? 03000u : pick(2u) ? 01400u : (uint16_t)(pick(8u) << 8);
    eb = (uint16_t)(w[3] >> 8);
    w[FIXLOC_AT] = (uint16_t)(0401u + 054u * pick(5u));
    if (n % 4u != 0u) {
        /* P11's restart words. */
        w[AT(LIFTTEMP)] = sign_word();
        w[AT(TLIFTOFF)] = pick(2u) ? 077764u /* REP11SA */ : sign_word();
        w[AT(PHASE5)] = sign_word();
        w[024] = sign_word();  /* TIME2 */
        w[025] = value_word(); /* TIME1 */
        /* The takeover's switch and FLAGWRD6's bits 15 and 14. */
        w[AT6(SATSW)] = sign_word();
        w[AT(FLAGWRD6)] = pick(4u) ? (uint16_t)(any_word() & 017777u) : any_word();
        w[AT6(RCSFLAGS)] = any_word();
        /* The boost's times. */
        w[AT6(RPSTART)] = near(w[025]);
        w[AT6(POLYSTOP)] = pick(2u) ? number(-(int)pick(020000u)) : value_word();
        w[AT6(POLYNUM)] = value_word();
        /* The attitude errors, their bias and scale. */
        for (k = 0; k < 3u; ++k) {
            w[AT6(AK) + k] = value_word();
            w[AT6(BIASAK) + k] = value_word();
            w[AT6(BOOSTEMP) + k] = value_word();
            w[MPAC_AT + k] = value_word();
        }
        w[AT6(SATSCALE)] = value_word();
        w[AT6(SATRLRT) + 1u] = value_word();
        /* The rate indexes. */
        w[AT6(RMANNDX)] = rate_index();
        w[AT6(PMANNDX)] = rate_index();
        w[AT6(YMANNDX)] = rate_index();
        for (k = 0; k < 3u; ++k) w[AT6(SATRATE) + k] = value_word();
        /* The channels. */
        io->channel12 = any_word();
        hw->channel30 = pick(2u) ? (uint16_t)(any_word() & ~01000u) : any_word();
        hw->channel31 = any_word();
        w[NEWJOB_AT] = pick(2u) ? 0u : (uint16_t)(rnd() & 037777u);
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
    if (interpretive) {
        pair_state(w, key, n);
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
            if (n % 64u != 63u) { /* now and then none: the basic routines refuse */
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
    /* Bank 34 */
    &P11_CODE,
    &MATRXJOB_CODE,
    &VHHDOT_CODE,
    &GETDOWN_CODE,
    &ATTDISP_CODE,
    &ATTDISP2_CODE,
    &STEERSAT_CODE,
    &S11_1_CODE,
    &NOPOLY_CODE,
    /* Bank 37 */
    &ATTDISP1_CODE,
};

int main(int argc, char **argv)
{
    unsigned i, failures = 0, runs = 0, basic = 0, words = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_P11_VECTORS; ++i)
        if (cm_p11_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&cm_p11_basic, &faithful_cm_p11_basic, "basic");
    failures += tables_differ(&cm_p11_interpretive, &faithful_cm_p11_interpretive, "interpretive");
    for (i = 0; i < faithful_cm_p11_basic.count; ++i)
        basic += cm_p11_readable(faithful_cm_p11_basic.entries[i].physical, false);
    if (basic != faithful_cm_p11_basic.count) { /* (every basic key) */
        printf("FAIL: %u basic keys readable of %u\n", basic, faithful_cm_p11_basic.count);
        ++failures;
    }
    for (i = 0; i < faithful_cm_p11_interpretive.count; ++i) {
        uint16_t key = faithful_cm_p11_interpretive.entries[i].physical;
        if (!cm_p11_readable(key, true)) {
            printf("FAIL: order pair %02o,%04o not readable\n", key / 1024u, 02000u + key % 1024u);
            ++failures;
        }
    }
    if (failures) {
        printf("cm-p11-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_p11_basic, &faithful_cm_p11_basic, false, &runs);
    failures += keys(&cm_p11_interpretive, &faithful_cm_p11_interpretive, true, &runs);
    printf("cm-p11-cases: %u words as the rope's, all %u keys readable (%u basic, %u order pairs) "
           "from %u states each, %u run by the faithful routines, %u failures\n",
           words, cm_p11_basic.count + cm_p11_interpretive.count, cm_p11_basic.count,
           cm_p11_interpretive.count, CASES, runs, failures);
    return failures != 0;
}
