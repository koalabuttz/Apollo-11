/* SPDX-License-Identifier: MIT */
/* The readable P51-P53 against the faithful port's, key by key (make
 * p51-p53-cases ROPE_LM ROPE_CM), the LM's.
 *
 * Built with the readable tables (src/binding/p51_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p51_p53/), written
 * in the listing's notation, must be the original program's rope, word for
 * word. Then the keys, which must be the faithful tables', and every key of
 * the parts written so far must run readable code. Then the runs: each
 * readable key from 4,096 states (each other key, which runs the faithful
 * routine through the readable table, from 16), each one of the port's
 * frozen test states (c64-port/tests/p51_p53_vectors.h, the LM's) entered
 * at the key, most stirred further: MPAC and its mode, the push-down list
 * and its pointer, the index registers (X1 and X2 as the program sets
 * them, or star numbers times 6 as R56 counts them, S1 any), A, Q (a
 * return address, or any), MODREG (often 52 or 57), OPTION2 (P52's
 * option, often 1 to 4), DSPTEM1 (often +0 or -0, the crew's time of
 * alignment not loaded), TIME2 and TIME1, the flag words (the REFSMMAT,
 * preferred attitude, lunar, earth, VFLAG and occultation flags: often one
 * bit changed), the vectors and times (any erasable bank's words), NEWJOB
 * (+0, a core set, or -0), the overflow indicator, now and then any word
 * but the interpreter's own, the latches, or a state the routines refuse
 * (another work area, BBANK not the two banks, a word of 16 bits). Each
 * runs once on each side, as the program's trampoline would call the key's
 * routine. Where either port runs, the other must too, with the same memory
 * and io record, word for word; where the readable one refuses, it must
 * leave everything as it found it; and the faithful routine must run in at
 * least 1 of 16 of each readable key's states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "p66_bias.h"
#include "p51_p53_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code P52E_CODE, P52V_CODE, P52W_CODE, P52H_CODE, REGCOARS_CODE,
    COARSTYP_CODE, P52LS_CODE, N89DISP_CODE, LSDISP_CODE, LOCSAM_CODE, PICAPAR_CODE, PICEND_CODE,
    PICGXT_CODE, R51_CODE, R51C_CODE, R51_3_CODE, R51_MARK_CODE, R51_SECOND_CODE, R51_TORQUE_CODE,
    R51P63_CODE, R51_4_CODE, GYCOARS_CODE, GYCOARS_DONE_CODE, R55_CODE, R55RET_CODE, CHKSDATA_CODE,
    CHKSDATA_ENTER_CODE, CHKSDA_CODE, CAL53A_CODE, COARFINE_CODE, MATMOVE_CODE;
extern const agc_routine_table lm_p51_p53_basic, lm_p51_p53_interpretive;
extern const agc_routine_table faithful_lm_p51_p53_basic, faithful_lm_p51_p53_interpretive;
bool p51_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define NEWOPS_AT   06070u
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
#define MODREG_AT   01011u
#define OPTION2_AT  01145u
#define DSPTEM1_AT  01045u
#define TIME2_AT    024u
/* The erasable words the routines' GOTOs and CALLs read a CADR from
 * (lm_p51_p53_cadr_words). */
static const uint16_t cadr_words[] = {00052u, 02736u, 02745u, 02746u, 03665u};

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 5152u;
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

/* The parts written so far: part 1, bank 15's P52 (PROG52 to P52LS) and
 * bank 14's N89DISP and LSDISP; part 2, bank 14's S50 and R56 (LOCSAM to
 * PICNSWP +1); part 3, R51 and GYCOARS (to its GOTO R51K). */
static bool in_written_part(uint16_t key)
{
    unsigned bank = key / 1024u, z = 02000u + key % 1024u;

    return (bank == 015u && z >= 02050u && z <= 02243u) ||
           (bank == 014u && z >= 02446u && z <= 03410u);
}

static unsigned parts_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (in_written_part(key) != p51_readable(key, order_pair)) {
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
static unsigned mission_cases[P51_P53_VECTORS], mission_count;

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

/* An index register: as the program sets it (index_word), a star's index
 * in the catalog (its number, 1 to 38, times 6), -5 to 243, one that
 * takes CATLOG's address (030346, the address word less one) into
 * erasable memory (the work area below 0100, or a bank's words, not so
 * near the bank's end that a vector would run past it; as a positive
 * index, or a negative one whose sum overflows), or into another fixed
 * bank (2 to 13). */
static uint16_t index_or_star(void)
{
    unsigned k = rnd() % 5u;

    if (k == 0u) return index_word();
    if (k == 1u) return (uint16_t)(6u * (1u + rnd() % 38u));
    if (k == 2u) {
        k = rnd() % 249u;
        return (uint16_t)(k < 5u ? 077777u - (k + 1u) : k - 5u);
    }
    if (k == 3u) {
        k = rnd() % 04000u;
        if ((k & 0377u) > 0371u) k -= 6u;
        return (uint16_t)(rnd() % 2u ? 030346u - k : ~(07432u + k) & 077777u);
    }
    return (uint16_t)(030346u - (04000u + rnd() % 024000u));
}

/* An index register for part 4's indexed operands (CHKSB's marks,
 * CALOOP's angles, MATMOVE's matrices), which the program only ever points
 * into erasable memory: at CALOOP the count, 1 to 3 (AXT,1 3; TIX,1);
 * elsewhere as the program sets it (index_word), or the complement of any
 * place from 0400 to 03377 not so near the end of its 256 words that a
 * matrix would run past them. (An index that takes such an operand into
 * fixed memory, or a multi-word one across the end of 256 words, where
 * the AGC's indexed address goes on into fixed memory, reads the rope on
 * the readable side, where the faithful port has no copy of it; one that
 * stores over the interpreter's own words, below 0400, no program does.) */
static uint16_t index_erasable(bool caloop)
{
    unsigned place;

    if (caloop) return (uint16_t)(1u + rnd() % 3u);
    if (rnd() % 2u) return index_word();
    place = 0400u + rnd() % (03400u - 0400u);
    if ((place & 0377u) > 0355u) place -= 022u;
    return (uint16_t)(~place & 077777u);
}

/* A fixed CADR (as STQ leaves one): a chain of erasable CADRs that never
 * reaches a fixed one loops in the AGC until the night watchman restarts
 * it, and the faithful port's host build stops the test there. */
static uint16_t fixed_cadr(uint16_t now)
{
    return (now & 074000u) ? now : (uint16_t)(04000u + rnd() % 074000u);
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
static void state(apollo_overlay_state *s, p66_bias_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &p51_p53_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = key_z(key), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) io->superbank &= (uint16_t)~0100u; /* banks 30-37 as such */
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
        /* A, Q. */
        w[0] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 2u) w[2] = return_word();
        /* MODREG: P52, P57, P00, or any. */
        k = rnd() % 4u;
        w[MODREG_AT] = k == 0u ? 064u : k == 1u ? 071u : k == 2u ? 0u : any_word();
        /* OPTION2: P52's option (1 to 4, the low two bits), or any. */
        if (rnd() % 2u) w[OPTION2_AT] = rnd() % 2u ? (uint16_t)(1u + rnd() % 4u) : any_word();
        /* DSPTEM1: the crew's time of alignment, often not loaded. */
        k = rnd() % 4u;
        if (k == 0u)
            w[DSPTEM1_AT] = w[DSPTEM1_AT + 1u] = 0u;
        else if (k == 1u)
            w[DSPTEM1_AT] = 077777u;
        /* TIME2 and TIME1. */
        if (rnd() % 2u) {
            w[TIME2_AT] = any_word();
            w[TIME2_AT + 1u] = any_word();
        }
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
    /* X1 and X2 (the work area's 046 and 047), each as the program sets
     * it, a star number times 6 as R56 counts them through the star
     * catalog (CATLOG,1 and CATLOG,2), -5 to 243 round the ends of the
     * faithful port's copy of the catalog, or into erasable memory (part
     * 4's from index_erasable); S1 (050) any. */
    if (key / 1024u == 014u && key % 1024u + 02000u >= 03224u) {
        bool caloop = key % 1024u + 02000u == 03340u || key % 1024u + 02000u == 03342u;
        w[(fixloc + X1_AT) & 03777u] = index_erasable(caloop);
        w[(fixloc + X1_AT + 1u) & 03777u] = index_erasable(caloop);
    } else {
        w[(fixloc + X1_AT) & 03777u] = index_or_star();
        w[(fixloc + X1_AT + 1u) & 03777u] = index_or_star();
    }
    if (n % 4u != 0u && rnd() % 2u) w[(fixloc + S1_AT) & 03777u] = any_word();
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    for (k = 0; k < sizeof cadr_words / sizeof cadr_words[0]; ++k)
        w[cadr_words[k]] = fixed_cadr(w[cadr_words[k]]);
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = key_z(key);
        bool written = p51_readable(key, interpretive != 0);
        unsigned ran = 0, bad = 0, cases = written ? CASES : OTHER_CASES;

        *readable_keys += written;
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            p66_bias_io io, fio, rio;
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
            fp.hardware = &fhw;
            rp.hardware = &rhw;
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
                printf("FAIL %02o,%04o case %u: %s\n", key / 1024u, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (written && ran < cases / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states\n", key / 1024u, z,
                   ran, cases);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

static const struct interpretive_code *const codes[] = {
    &P52E_CODE,     &P52V_CODE,           &P52W_CODE,         &P52H_CODE,       &REGCOARS_CODE,
    &COARSTYP_CODE, &P52LS_CODE,          &N89DISP_CODE,      &LSDISP_CODE,     &LOCSAM_CODE,
    &PICAPAR_CODE,  &PICEND_CODE,         &PICGXT_CODE,       &R51_CODE,        &R51C_CODE,
    &R51_3_CODE,    &R51_MARK_CODE,       &R51_SECOND_CODE,   &R51_TORQUE_CODE, &R51P63_CODE,
    &R51_4_CODE,    &GYCOARS_CODE,        &GYCOARS_DONE_CODE, &R55_CODE,        &R55RET_CODE,
    &CHKSDATA_CODE, &CHKSDATA_ENTER_CODE, &CHKSDA_CODE,       &CAL53A_CODE,     &COARFINE_CODE,
    &MATMOVE_CODE,
};
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0, readable_basic = 0, readable_pairs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < P51_P53_VECTORS; ++i)
        if (p51_p53_cases[i].cm == 0u) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&lm_p51_p53_basic, &faithful_lm_p51_p53_basic, "basic");
    failures +=
        tables_differ(&lm_p51_p53_interpretive, &faithful_lm_p51_p53_interpretive, "interpretive");
    failures += parts_unwritten(&faithful_lm_p51_p53_basic, false);
    failures += parts_unwritten(&faithful_lm_p51_p53_interpretive, true);
    if (failures) {
        printf("p51-p53-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&lm_p51_p53_basic, &faithful_lm_p51_p53_basic, 0, &runs, &readable_basic);
    failures += keys(&lm_p51_p53_interpretive, &faithful_lm_p51_p53_interpretive, 1, &runs,
                     &readable_pairs);
    printf("p51-p53-cases: %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each (the others from %u), %u run by the faithful routines, "
           "%u failures\n",
           words, readable_basic + readable_pairs,
           lm_p51_p53_basic.count + lm_p51_p53_interpretive.count, readable_basic, readable_pairs,
           CASES, OTHER_CASES, runs, failures);
    return failures != 0;
}
