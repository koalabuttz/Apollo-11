/* SPDX-License-Identifier: MIT */
/* The readable CM P20-P25 against the faithful port's, key by key (make
 * cm-p20-p25-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_p20_p25_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p20_p25_cm/),
 * written in the listing's notation, must be the original program's rope,
 * word for word. Then the keys, which must be the faithful tables', and
 * every key of the parts written so far must run readable code. Then the
 * runs: each readable key from 4,096 states (each other key, which runs
 * the faithful routine through the readable table, from 16), each one of
 * the port's frozen test states (c64-port/tests/cm_p20_p25_vectors.h, with
 * its channels) entered at the key, most stirred further: MPAC and its
 * mode, the push-down list and its pointer, the index registers (X1 and X2
 * as the program sets them, S1 any), A, Q (a return address, or any), the
 * flag words (often one bit changed: P20's track flag and REFSMMAT flag
 * among them), the vectors and times (any erasable bank's words), NEWJOB,
 * the overflow indicator, now and then any word but the interpreter's own,
 * the latches, or a state the routines refuse (another work area, BBANK
 * not the two banks, a word of 16 bits, no hardware record). Each runs
 * once on each side, as the program's trampoline would call the key's
 * routine. Where either port runs, the other must too, with the same
 * memory, io record and hardware record, word for word; where the readable
 * one refuses, it must leave everything as it found it; and the faithful
 * routine must run from at least 1 in 16 of the states of every readable
 * key. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_p20_p25_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code PROG20_CODE, P20R52JB_CODE, PROG22_CODE, PROG22A_CODE,
    CALLR52_CODE, S22LSITE_CODE, P22SUBRB_CHECK_CODE, N89ERRX_3_CODE, S22ABDE_CODE, DE_GR_50_CODE,
    R57_CODE, ENDR57_CODE, R61CSM_CODE, BVECTORS_CODE, GETUM_CODE, R22_CODE, REND2_CODE, REND3_CODE,
    REND4_CODE, REND6_CODE, REND9_CODE, REND10_CODE, REND10A_CODE, REND12_CODE, REND14_CODE,
    CMPOS_CODE, RENDISP_CODE, RENDISP7_CODE, RENDISP3_CODE, RENDISP4_CODE, SETINTG_CODE,
    CNTCHK_CODE, WAITONE_CODE, WAITONE_AGAIN_CODE, RANGERD_CODE, RANGERD4_CODE, RANGERD3_CODE,
    LIGHTON_CODE, SHIFTNDX_CODE, INITIALW_CODE;
extern const agc_routine_table cm_p20_p25_basic, cm_p20_p25_interpretive;
extern const agc_routine_table faithful_cm_p20_p25_basic, faithful_cm_p20_p25_interpretive;
bool cm_p20_p25_readable(uint16_t key, bool order_pair);

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
#define S22TOFF_AT  03667u /* E7,1667: S22's return, a CADR */
#define MRKBUF1_AT  03725u /* E7,1725 */
#define EGRESS_AT   02317u /* E4,1717: the return of R57, BVECTORS and GETUM, a CADR */
#define MARKDATA_AT 01242u /* the address of the mark data GETUM takes */
#define MARKDOWN_AT 03674u /* E7,1674: GETUM's copy of the mark data, 7 words */

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 2020u;
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

/* The parts written so far: P20, R57, R21, R23 and R61CSM (bank 37,
 * parts 1, 3 and 4); P22 up to S22.1 (bank 30, part 2); BVECTORS and
 * GETUM (bank 23, part 4); R22 (bank 34, parts 5 and 6); GOTOV56
 * (fixed-fixed, part 6); R22's subroutines (bank 27, part 7). */
static bool in_written_part(uint16_t key)
{
    unsigned bank = key / 1024u, z = 02000u + key % 1024u;

    return (bank == 037u && z >= 02261u && z <= 02624u) ||
           (bank == 030u && z >= 02000u && z <= 02311u) ||
           (bank == 023u && z >= 03043u && z <= 03131u) ||
           (bank == 034u && z >= 02630u && z <= 03334u) || (bank == 02u && z == 02622u) ||
           (bank == 027u && z >= 02343u && z <= 02613u);
}

static unsigned parts_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (in_written_part(key) != cm_p20_p25_readable(key, order_pair)) {
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
static unsigned mission_cases[CM_P20_P25_VECTORS], mission_count;

/* A return address (a TC's), or any word. */
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

/* The address of a key: 02000-03777 in its bank, or 04000-07777 in
 * fixed-fixed memory (banks 2 and 3). */
static uint16_t key_z(uint16_t key)
{
    unsigned bank = key / 1024u;

    return (uint16_t)(bank == 2u || bank == 3u ? 04000u + 02000u * (bank - 2u) + key % 1024u
                                               : 02000u + key % 1024u);
}

/* A CADR in fixed memory, as STQ leaves one: mostly the word now if it
 * is one, else any. A GOTO through an erasable word takes the CADR there. */
static uint16_t fixed_cadr(uint16_t now)
{
    return (now & 074000u) ? now : (uint16_t)(04000u + rnd() % 074000u);
}

/* The FB of a key's bank (banks 40-43 are FB 30-33, with the superbank
 * bit), and the superbank bit it wants. */
static uint16_t key_fb(uint16_t key)
{
    unsigned bank = key / 1024u;

    return (uint16_t)((bank >= 040u ? bank - 010u : bank) << 10);
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hardware, uint16_t key,
                  int interpretive, unsigned n)
{
    const irc_case *c = &cm_p20_p25_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = key_z(key), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hardware, 0, sizeof *hardware);
    irc_build(s, c);
    if (irc_ch_on) irc_cm_io(io, hardware, 0);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    /* Banks 30-37 as such, and 40-43 with the superbank bit. */
    if (rnd() % 4u) {
        if (bank >= 040u)
            io->superbank |= 0100u;
        else
            io->superbank &= (uint16_t)~0100u;
    }
    eb = (uint16_t)(w[3] >> 8);
    w[4] = (uint16_t)(bank < 4u ? w[4] : key_fb(key));
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
        /* The vectors and times: any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A, Q. */
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
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    /* X1 and X2 (the work area's 046 and 047); S1 (050) any. */
    w[(fixloc + X1_AT) & 03777u] = index_word();
    w[(fixloc + X1_AT + 1u) & 03777u] = index_word();
    if (n % 4u != 0u && rnd() % 2u) w[(fixloc + S1_AT) & 03777u] = any_word();
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* S22TOFF and S22TOFF +1, the returns P22's GOTOs take, CADRs in fixed
     * memory as the STQs leave them. */
    if (rnd() % 16u) {
        w[S22TOFF_AT] = fixed_cadr(w[S22TOFF_AT]);
        w[S22TOFF_AT + 1u] = fixed_cadr(w[S22TOFF_AT + 1u]);
    }
    /* EGRESS, the return GOTO EGRESS takes, a CADR in fixed memory as the
     * STQs leave it. (One in erasable memory could send the faithful GOTO
     * round a cycle of CADRs, which it ends by exiting.) */
    w[EGRESS_AT] = fixed_cadr(w[EGRESS_AT]);
    /* Now and then MRKBUF1 -1, no mark (R21CSMA). */
    if (rnd() % 4u == 0u) w[MRKBUF1_AT] = 077776u;
    /* Now and then MPAC +0 or 1, CRS61.1's answers (R61CSM +015). */
    if (rnd() % 4u == 0u) w[MPAC_AT] = (uint16_t)(rnd() % 2u);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* The interrupt latch either way now and then. */
    if (n % 4u != 0u && rnd() % 8u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 2u);
}

/* GETUM's load of the mark data (VLOAD* 1,1, X1 the complement of
 * MARKDATA): the operand 1 is in the work area, so ADDRWD is FIXLOC +
 * MARKDATA, and one of 01772-01777 puts the vector's six words across the
 * end of the E-bank window (01777). The AGC reads the words past it from
 * fixed memory, and so does the readable machine; the faithful port's
 * machine reads the unswitched words 0-4 instead (ai_ea keeps the address in
 * the erasable image). The test counts apart the states where a pass makes
 * that load: from GETUM +3, or on into MARKDOWN (written, on either side). */
static unsigned wraps;

static bool load_wraps(uint16_t key, const apollo_overlay_state *start,
                       const apollo_overlay_state *fa, const apollo_overlay_state *ra)
{
    const uint16_t *w = start->words;
    unsigned addrwd = (w[FIXLOC_AT] & 077777u) + (w[MARKDATA_AT] & 077777u);
    size_t n = 7u * sizeof(uint16_t);

    if (addrwd < 01772u || addrwd > 01777u) return false;
    if (key != 023u * 1024u + 01107u /* GETUM +3 */ &&
        !memcmp(&fa->words[MARKDOWN_AT], &w[MARKDOWN_AT], n) &&
        !memcmp(&ra->words[MARKDOWN_AT], &w[MARKDOWN_AT], n))
        return false;
    ++wraps;
    return true;
}

/* INITIALW's stores, STORE W +36D,1 and W +90D,1, in TIX loops by S1:
 * from INITA or INITB, with an X1 and S1 the program never has there, a
 * store's six words can cross the end of the E-bank window (01777) the
 * same way. ADDRWD is the address less one, less X1 (ones' complement),
 * in the E bank it names, words 01400 + its low 8 bits on (not the work
 * area, nor fixed). The test follows the loops (TIX: X1 less S1 while
 * that is positive) and counts those states apart. */
static bool crosses(unsigned operand, unsigned x1)
{
    unsigned addrwd = operand - 1u + (~x1 & 077777u);

    if (addrwd > 077777u) addrwd = (addrwd + 1u) & 077777u;
    if (!(addrwd & 077700u) || (addrwd & 074000u)) return false;
    return (addrwd & 0377u) + 5u > 0377u;
}

static unsigned sign_extended(unsigned v)
{
    v &= 077777u;
    return v & 040000u ? v | 0100000u : v;
}

static bool loop_crosses(unsigned operand, unsigned x1, unsigned s1)
{
    unsigned n, a;

    for (n = 0; n < 512u; ++n) {
        if (crosses(operand, x1)) return true;
        a = sign_extended(~s1) + sign_extended(x1); /* CS S1; AD X1 */
        if (a > 0177777u) a = (a + 1u) & 0177777u;
        if (a == 0u || a == 0177777u || (a & 0100000u)) return false;
        x1 = a & 077777u;
    }
    return false;
}

static bool store_wraps(uint16_t key, const apollo_overlay_state *start)
{
    const uint16_t *w = start->words;
    unsigned fixloc = w[FIXLOC_AT];
    unsigned x1 = w[(fixloc + X1_AT) & 03777u], s1 = w[(fixloc + S1_AT) & 03777u];
    bool wraps_here;

    if (key == 027u * 1024u + 0573u) /* INITA, then INITB from X1 36 */
        wraps_here = loop_crosses(02445u, x1, s1) || loop_crosses(02533u, 36u, s1);
    else if (key == 027u * 1024u + 0577u) /* INITB */
        wraps_here = loop_crosses(02533u, x1, s1);
    else
        return false;
    wraps += wraps_here;
    return wraps_here;
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = key_z(key);
        bool written = cm_p20_p25_readable(key, interpretive != 0);
        unsigned ran = 0, bad = 0, cases = written ? CASES : OTHER_CASES;

        *readable_keys += written;
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
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
            /* Now and then no hardware record, which the routines refuse. */
            if (n % 4u == 0u || rnd() % 64u) {
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (written && (load_wraps(key, &start, &fa, &ra) || store_wraps(key, &start))) {
                /* (counted apart) */
            } else if (!written) { /* the faithful routine on both sides, refusals and all */
                if (sf != sr || memcmp(&fa, &ra, sizeof fa) || memcmp(&fio, &rio, sizeof fio) ||
                    memcmp(&fhw, &rhw, sizeof fhw))
                    why = "the table does not run the faithful routine";
            } else if (sr != EXEC_OK &&
                       (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
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
        /* Each written key's faithful routine must run in at least 1 of
         * 16 states, so that a change to the states cannot quietly leave
         * a key with a handful of live ones. */
        if (written && ran < cases / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states (fewer than 1 in "
                   "16)\n",
                   key / 1024u, z, ran, cases);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

static const struct interpretive_code *const codes[] = {
    &PROG20_CODE,   &P20R52JB_CODE,       &PROG22_CODE,    &PROG22A_CODE,       &CALLR52_CODE,
    &S22LSITE_CODE, &P22SUBRB_CHECK_CODE, &N89ERRX_3_CODE, &S22ABDE_CODE,       &DE_GR_50_CODE,
    &R57_CODE,      &ENDR57_CODE,         &R61CSM_CODE,    &BVECTORS_CODE,      &GETUM_CODE,
    &R22_CODE,      &REND2_CODE,          &REND3_CODE,     &REND4_CODE,         &REND6_CODE,
    &REND9_CODE,    &REND10_CODE,         &REND10A_CODE,   &REND12_CODE,        &REND14_CODE,
    &CMPOS_CODE,    &RENDISP_CODE,        &RENDISP7_CODE,  &RENDISP3_CODE,      &RENDISP4_CODE,
    &SETINTG_CODE,  &CNTCHK_CODE,         &WAITONE_CODE,   &WAITONE_AGAIN_CODE, &RANGERD_CODE,
    &RANGERD4_CODE, &RANGERD3_CODE,       &LIGHTON_CODE,   &SHIFTNDX_CODE,      &INITIALW_CODE,
};
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0, readable_basic = 0, readable_pairs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_P20_P25_VECTORS; ++i)
        if (cm_p20_p25_cases[i].cm) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&cm_p20_p25_basic, &faithful_cm_p20_p25_basic, "basic");
    failures +=
        tables_differ(&cm_p20_p25_interpretive, &faithful_cm_p20_p25_interpretive, "interpretive");
    failures += parts_unwritten(&faithful_cm_p20_p25_basic, false);
    failures += parts_unwritten(&faithful_cm_p20_p25_interpretive, true);
    if (failures || !mission_count) {
        printf("cm-p20-p25-cases: %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&cm_p20_p25_basic, &faithful_cm_p20_p25_basic, 0, &runs, &readable_basic);
    failures += keys(&cm_p20_p25_interpretive, &faithful_cm_p20_p25_interpretive, 1, &runs,
                     &readable_pairs);
    printf("cm-p20-p25-cases: %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each (the others from %u), %u run by the faithful routines, "
           "%u loads and stores across 01777 counted apart, %u failures\n",
           words, readable_basic + readable_pairs,
           cm_p20_p25_basic.count + cm_p20_p25_interpretive.count, readable_basic, readable_pairs,
           CASES, OTHER_CASES, runs, wraps, failures);
    return failures != 0;
}
