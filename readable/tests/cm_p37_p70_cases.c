/* SPDX-License-Identifier: MIT */
/* The readable P37_P70 of the CM against the faithful port's, key by key
 * (make cm-p37-p70-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_p37_p70_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p37_p70/), written
 * in the listing's notation, must be the original program's rope, word
 * for word. Then the keys, which must be the faithful tables', and every
 * key of the parts written so far must run readable code. Then the runs:
 * each readable key from 4,096 states (each other key, which runs the
 * faithful routine through the readable table, from 16), each one of the
 * port's frozen test states (c64-port/tests/cm_p37_p70_vectors.h) entered
 * at the key, most stirred further: MPAC and its mode, the push-down list
 * and its pointer, the index registers, A and Q, SPRTEX and RTENCKEX
 * (often one of the file's display returns, or a word no TC would leave),
 * EBANK (often E7, the file's), the flag words (often one bit changed),
 * the vectors and times (any erasable bank's words), NEWJOB (+0 or a core
 * set, now and then -0), the overflow indicator, now and then any word
 * but the interpreter's own, the latches, or a state the routines refuse
 * (another work area, BBANK not the two banks, a word of 16 bits, the
 * superbank bit set). Each runs once on each side, as the program's
 * trampoline would call the key's routine, with a hardware record or (now
 * and then) none. Where either port runs, the other must too, with the
 * same memory and io record, word for word; where the readable one
 * refuses, it must leave everything as it found it; and the faithful
 * routine must run in at least 1 of 16 of each readable key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_p37_p70_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code P37_CODE, RTE299_CODE, RTE317_POLY_CODE, RTE369_POLY_CODE,
    RTEVN_CODE, RTEVN_RETURN_CODE, P37N_CODE, P37T_POLY_CODE, P37T_TIG_CODE;
extern const struct interpretive_code P370ALRM_CODE, P370ALRM_GOTO_CODE, TMRAD100_CODE, RTD18_CODE;
extern const struct interpretive_code INVC100_CODE, PREC100_CODE, PREC130_CODE, PREC175_CODE,
    PREC220_CODE;
extern const struct interpretive_code RTENCK_CODE, V2T100_CODE, V2T101_CODE, V2T102_CODE;
extern const agc_routine_table cm_p37_p70_basic, cm_p37_p70_interpretive;
extern const agc_routine_table faithful_cm_p37_p70_basic, faithful_cm_p37_p70_interpretive;
bool cm_p37_p70_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define NEWOPS_AT   06037u
#define X1_AT       0046u
#define S1_AT       0050u
#define EDOP_AT     0023u
#define NEWJOB_AT   0067u
#define FLAGWRD0_AT 0074u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define OVFIND_AT   0121u
#define MPAC_AT     0154u
#define MODE_AT     0163u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
/* SPRTEX and RTENCKEX (E7,1762 and E7,1733, physical), where the display
 * subroutines keep their returns, and the returns the file leaves there
 * (the words after its TCRs of P370GOF, P37GFRB1 and P370GOFR). */
#define SPRTEX_AT   03762u
#define RTENCKEX_AT 03733u
/* VNSTORE (E7,1763), where RTEVN keeps its return (STQ) and goes back
 * through (GOTO). */
#define VNSTORE_AT 03763u
static const uint16_t display_returns[] = {02503u, 02506u, 03010u, 03016u, 03021u,
                                           03024u, 03041u, 03120u, 03214u};

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 890u;
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

/* The parts written so far: part 1, bank 36's P37 to V6N81RTE. */
static bool in_written_part(uint16_t key)
{
    unsigned bank = key / 1024u, z = 02000u + key % 1024u;

    return (bank == 036u && z >= 02466u && z <= 03253u) ||
           (bank == 032u && z >= 02262u && z <= 03456u) ||
           (bank == 031u && z >= 03700u && z <= 03760u) ||
           (bank == 016u && z >= 02703u && z <= 02714u);
}

static unsigned parts_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (in_written_part(key) != cm_p37_p70_readable(key, order_pair)) {
            printf("FAIL: %02o,%04o %s readable\n", key / 1024u, 02000u + key % 1024u,
                   in_written_part(key) ? "is not" : "is");
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
    return rnd() % 4u ? any_word() : edges[rnd() % (sizeof edges / sizeof edges[0])];
}

/* The frozen cases. */
static unsigned mission_cases[CM_P37_P70_VECTORS], mission_count;

/* A return address: a TC's, or any. */
static uint16_t return_word(void)
{
    return rnd() % 4u ? (uint16_t)(010u + rnd() % 07770u) : rnd() % 2u ? any_word() : rnd();
}

/* X1 or X2 as the program sets it: the complement of a place in erasable
 * memory, which an indexed operand adds. */
static uint16_t index_word(void)
{
    return (uint16_t)(077777u - (0400u + rnd() % 0340u));
}

/* A place in fixed memory, as an interpretive GOTO goes on at it: its
 * address in a bank (02000-03777, with the bank's low bits above it), or
 * in fixed-fixed memory. */
static uint16_t fixed_place(void)
{
    return (uint16_t)(04000u + rnd() % 074000u);
}

/* The address of a key: 02000-03777 in its bank, or 04000-07777 in
 * fixed-fixed memory (banks 2 and 3). */
static uint16_t key_z(uint16_t key)
{
    unsigned bank = key / 1024u;

    return (uint16_t)(bank == 2u || bank == 3u ? 04000u + 02000u * (bank - 2u) + key % 1024u
                                               : 02000u + key % 1024u);
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &cm_p37_p70_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = key_z(key), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) io->superbank &= (uint16_t)~0100u; /* banks 30-37 as such */
    if (rnd() % 4u) w[3] = rnd() % 4u ? 03400u : (uint16_t)(rnd() % 8u << 8); /* often E7 */
    eb = (uint16_t)(w[3] >> 8);
    w[4] = (uint16_t)(bank < 4u ? w[4] : bank << 10);
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
        k = rnd() % 4u;
        if (k == 0u)
            w[MPAC_AT] = (uint16_t)(rnd() % 2u);
        else if (k == 1u)
            w[MPAC_AT] = rnd() % 2u ? 077777u : 077776u;
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
        /* The vectors and times: any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A and Q. */
        w[0] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 2u) w[2] = return_word();
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
        if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 32u == 0u) w[INTBIT15_AT] = any_word();
        if (rnd() % 64u == 0u) w[3] = any_word();
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
        w[PUSHLOC_AT] = (uint16_t)((fixloc + rnd() % 040u) & 077777u);
    /* X1 and X2 (the work area's 046 and 047), each as the program sets
     * it; S1 (050) any. */
    w[(fixloc + X1_AT) & 03777u] = index_word();
    w[(fixloc + X1_AT + 1u) & 03777u] = index_word();
    if (n % 4u != 0u && rnd() % 2u) w[(fixloc + S1_AT) & 03777u] = any_word();
    /* 20D, 31D, 32D and 33D, where the file's subroutines keep their
     * returns (STQ) and go back through (GOTO), fixed CADRs: an erasable
     * one may name a cycle of erasable CADRs, which the faithful
     * interpreter stops the test at. */
    for (k = 0; k < 4u; ++k) w[(fixloc + (k ? 036u + k : 024u)) & 03777u] = fixed_place();
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* SPRTEX and RTENCKEX: often one of the file's display returns, now
     * and then a word no TC leaves (another instruction, or a register's
     * address). */
    for (k = 0; k < 2u; ++k) {
        uint16_t *at = &w[k ? RTENCKEX_AT : SPRTEX_AT];
        unsigned r = rnd() % 8u;
        if (r < 5u)
            *at = display_returns[rnd() % (sizeof display_returns / sizeof display_returns[0])];
        else if (r == 5u)
            *at = (uint16_t)(rnd() % 010u);
        else if (r == 6u)
            *at = any_word();
        /* (An order pair may go on through it, GOTO or STCALL: see
         * VNSTORE. To the interpreter any address below 04000 is an
         * erasable one, the display returns too.) */
        if (interpretive && (*at & 077777u) < 04000u) *at = fixed_place();
    }
    /* VNSTORE, where RTEVN's GOTO goes back, a place in fixed memory: an
     * erasable one may name a cycle of erasable addresses, which the
     * faithful interpreter stops the test at. */
    w[VNSTORE_AT] = fixed_place();
    /* Now and then no hardware record: see keys(). */
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = key_z(key);
        bool written = cm_p37_p70_readable(key, interpretive != 0);
        unsigned ran = 0, bad = 0, cases = written ? CASES : OTHER_CASES;

        *readable_keys += written;
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            memset(&hardware, 0, sizeof hardware);
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            if (n % 64u != 63u) { /* now and then none */
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (!written) { /* the faithful routine on both sides, refusals and all */
                if (sf != sr || memcmp(&fa, &ra, sizeof fa) || memcmp(&fio, &rio, sizeof fio))
                    why = "the table does not run the faithful routine";
            } else if (sr != EXEC_OK &&
                       (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
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
                printf("FAIL %02o,%04o case %u: %s\n", key / 1024u % 64u, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (written && ran < cases / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states\n",
                   key / 1024u % 64u, z, ran, cases);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

static const struct interpretive_code *const codes[] = {
    /* Part 1 */
    &P37_CODE,
    &RTE299_CODE,
    &RTE317_POLY_CODE,
    &RTE369_POLY_CODE,
    &RTEVN_CODE,
    &RTEVN_RETURN_CODE,
    &P37N_CODE,
    &P37T_POLY_CODE,
    &P37T_TIG_CODE,
    /* Part 2 */
    &P370ALRM_CODE,
    &P370ALRM_GOTO_CODE,
    &TMRAD100_CODE,
    &RTD18_CODE,
    /* Part 3 */
    &INVC100_CODE,
    &PREC100_CODE,
    &PREC130_CODE,
    &PREC175_CODE,
    &PREC220_CODE,
    /* Part 4 */
    &RTENCK_CODE,
    &V2T100_CODE,
    &V2T101_CODE,
    &V2T102_CODE,
};
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0, readable_basic = 0, readable_pairs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_P37_P70_VECTORS; ++i)
        if (cm_p37_p70_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&cm_p37_p70_basic, &faithful_cm_p37_p70_basic, "basic");
    failures +=
        tables_differ(&cm_p37_p70_interpretive, &faithful_cm_p37_p70_interpretive, "interpretive");
    failures += parts_unwritten(&faithful_cm_p37_p70_basic, false);
    failures += parts_unwritten(&faithful_cm_p37_p70_interpretive, true);
    if (failures) {
        printf("cm-p37-p70-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_p37_p70_basic, &faithful_cm_p37_p70_basic, 0, &runs, &readable_basic);
    failures += keys(&cm_p37_p70_interpretive, &faithful_cm_p37_p70_interpretive, 1, &runs,
                     &readable_pairs);
    printf("cm-p37-p70-cases: %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each, the others from %u, %u run by the faithful routines, "
           "%u failures\n",
           words, readable_basic + readable_pairs,
           cm_p37_p70_basic.count + cm_p37_p70_interpretive.count, readable_basic, readable_pairs,
           CASES, OTHER_CASES, runs, failures);
    return failures != 0;
}
