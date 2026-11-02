/* SPDX-License-Identifier: MIT */
/* The readable P20-P25 against the faithful port's, key by key (make
 * p20-p25-cases ROPE_LM ROPE_CM), the LM's.
 *
 * Built with the readable tables (src/binding/p20_p25_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p20_p25/), written
 * in the listing's notation, must be the original program's rope, word for
 * word. Then the keys, which must be the faithful tables', and every key of
 * the parts written so far must run readable code. Then the runs: each
 * readable key from 4,096 states (each other key, which runs the faithful
 * routine through the readable table, from 16), each one of the port's
 * frozen test states (c64-port/tests/lm_p20_p25_vectors.h) entered at the
 * key, most stirred further: MPAC and its mode (MPAC often 0, 1 or -0, as
 * LPS20.2 leaves it), the push-down list and its pointer, the index
 * registers (X1 and X2 as the program sets them, S1 any), A, Q (a return
 * address, or any), MODREG (often 20 or 22), OPTION2 (often 1), RADMODES,
 * the flag words (the rendezvous, track, surface, vehicle update, W matrix
 * and search flags: often one bit changed), channel 33 (its bit 2, the RR
 * auto mode discrete, often as P20 wants it), the vectors and times (any
 * erasable bank's words), NEWJOB, the overflow indicator, now and then any
 * word but the interpreter's own, the latches, or a state the routines
 * refuse (another work area, BBANK not the two banks, a word of 16 bits).
 * Each runs once on each side, as the program's trampoline would call the
 * key's routine. Where either port runs, the other must too, with the same
 * memory and io record, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * from some state of every readable key. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "lm_p20_p25_vectors.h"
#include "lm_interp_orders.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code ORBCHG2_CODE, P20LEM1_CODE, P20LEMC3_CODE, UPPSV_CODE,
    ORBCHG3_CODE, R22LEM3_CODE, R22LEM96_CODE, R22LEM96_WAIT_CODE, R22LEM7_CODE, R21LEM1_CODE,
    R21LEM3_CODE, R21LEM12_CODE, R21LEM12_FIRST_PASS_CODE, R21LEM7_CODE, R21LEM8_CODE,
    R61C_L03_CODE, R61LEM1_AUTO_CODE, R61C_L05_CODE;
extern const agc_routine_table lm_p20_p25_basic, lm_p20_p25_interpretive;
extern const agc_routine_table faithful_lm_p20_p25_basic, faithful_lm_p20_p25_interpretive;
bool p20_p25_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define NEWOPS_AT        06070u
#define DANZIG_AT        06061u
#define PENDING_CALL_KEY (023u * 1024u + 0150u) /* 23,2150 */
#define X1_AT            0046u
#define S1_AT            0050u
#define EDOP_AT          0023u
#define NEWJOB_AT        0067u
#define FLAGWRD0_AT      0074u
#define RADMODES_AT      0110u
#define INTBIT15_AT      0115u
#define FIXLOC_AT        0120u
#define OVFIND_AT        0121u
#define MPAC_AT          0154u
#define MODE_AT          0163u
#define LOC_AT           0164u
#define BANKSET_AT       0165u
#define PUSHLOC_AT       0166u
#define MODREG_AT        01011u
#define OPTION2_AT       01145u
#define REPOSCNT_AT      01423u
#define N49FLAG_AT       01746u
/* The erasable words UPPSV's GOTO and the faithful port read a CADR from:
 * LS21X (its return, as STQ stores it) and 03675. */
#define LS21X_AT 01757u
#define CADR2_AT 03675u

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

/* The parts written so far: parts 1 to 4, bank 24 to R24LEM4, bank 32's
 * ORBCHG3 (to its EXIT's TC BANKCALL), CLRADMOD in fixed-fixed and bank
 * 23's R61 (to READCDUD). */
static bool in_written_part(uint16_t key)
{
    unsigned bank = key / 1024u, z = 02000u + key % 1024u;

    return (bank == 024u && z < 03255u) || (bank == 032u && z >= 02217u && z <= 02364u) ||
           (bank == 03u && z == 02011u) /* CLRADMOD */ ||
           (bank == 023u && z >= 02116u && z <= 02313u);
}

static unsigned parts_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (in_written_part(key) != p20_p25_readable(key, order_pair)) {
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
static unsigned mission_cases[LM_P20_P25_VECTORS], mission_count;

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
static void state(apollo_overlay_state *s, lm_channel_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &lm_p20_p25_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = key_z(key), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) io->io.superbank &= (uint16_t)~0100u; /* banks 30-37 as such */
    eb = (uint16_t)(w[3] >> 8);
    w[4] = (uint16_t)(bank < 4u ? w[4] : bank << 10);
    if (interpretive) {
        w[5] = NEWOPS_AT;
        w[LOC_AT] = z;
        w[INTBIT15_AT] = (uint16_t)(w[4] & 040000u);
        w[EDOP_AT] = 0u; /* as DANZIG leaves it before every NEWOPS */
        /* R61LEM1's RTB CALL: the CALL waits in EDOP while READCDUD runs,
         * and DANZIG resumes the pair at LOC. */
        if (key == PENDING_CALL_KEY && rnd() % 8u) {
            w[5] = DANZIG_AT;
            w[EDOP_AT] = rnd() % 8u ? 0153u : (uint16_t)(rnd() % 0200u);
        }
    } else {
        w[5] = z;
    }
    fixloc = w[FIXLOC_AT];
    if (n % 4u != 0u) {
        fixloc = w[FIXLOC_AT] = (uint16_t)(0401u + 054u * (rnd() % 5u));
        for (k = 0; k < 7u; ++k) w[MPAC_AT + k] = value_word();
        k = rnd() % 4u;
        if (k == 0u)
            w[MPAC_AT] = (uint16_t)(rnd() % 3u); /* LPS20.2's, LRS22.1's answer */
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
        if (rnd() % 2u) w[RADMODES_AT] = rnd() % 2u ? any_word() : (uint16_t)(1u << (rnd() % 15u));
        /* The vectors and times: any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A, Q. */
        w[0] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 2u) w[2] = return_word();
        /* MODREG: P20, P22, P00, or any. */
        k = rnd() % 4u;
        w[MODREG_AT] = k == 0u ? 024u : k == 1u ? 026u : k == 2u ? 0u : any_word();
        if (rnd() % 2u) w[OPTION2_AT] = rnd() % 2u ? 1u : any_word();
        /* R21's count of tries: 0, a few, -0 or any; N49DSP's answer: -1
         * (proceed), 0 (none yet) or any. */
        k = rnd() % 4u;
        w[REPOSCNT_AT] = k == 0u   ? 0u
                         : k == 1u ? (uint16_t)(rnd() % 3u)
                         : k == 2u ? 077777u
                                   : any_word();
        k = rnd() % 3u;
        w[N49FLAG_AT] = k == 0u ? 077776u : k == 1u ? 0u : any_word();
        /* Channel 33: bit 2 (RR auto mode) clear as P20 wants it, or any. */
        k = rnd() % 4u;
        if (k == 0u)
            io->in.channel33 &= (uint16_t)~2u;
        else if (k == 1u)
            io->in.channel33 |= 2u;
        else if (k == 2u)
            io->in.channel33 = any_word();
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
        if (rnd() % 16u == 0u) io->io.superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 3u);
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
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    w[LS21X_AT] = fixed_cadr(w[LS21X_AT]);
    w[CADR2_AT] = fixed_cadr(w[CADR2_AT]);
}

/* R21LEM12's CLRGO (24,3030) branches to R21LEM7 +1, a TC INTPRET the
 * interpreter reads as a store code, STORE,1. Where that store would run
 * past the end of the E-bank window, the AGC writes into fixed memory and
 * reads it back into A and L: the readable binding refuses it, as the
 * faithful port's other interpretive routines do (lmi_store_spills), but
 * the faithful 24,3030 runs it, writing the next E bank's words. (CLRGO
 * changes neither MODE nor X1.) */
static unsigned spills;

static bool faithful_spills(uint16_t key, const apollo_overlay_state *start)
{
    bool spill = key == 024u * 1024u + 01030u && lmi_store_spills(start, 006037u);

    spills += spill;
    return spill;
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = key_z(key);
        bool written = p20_p25_readable(key, interpretive != 0);
        unsigned ran = 0, bad = 0, cases = written ? CASES : OTHER_CASES;

        *readable_keys += written;
        for (n = 0; n < cases; ++n) {
            apollo_overlay_state start, fa, ra;
            lm_channel_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            memset(&hardware, 0, sizeof hardware);
            /* Channel 12: the RR auto track enable (bit 14) on or off, or
             * any. */
            hardware.channel12 = rnd() % 4u
                                     ? (uint16_t)((rnd() & 077777u) | (rnd() % 2u ? 020000u : 0u))
                                     : (uint16_t)(rnd() % 2u ? 020000u : 0u);
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
                why = faithful_spills(key, &start) ? 0 : "readable refused, faithful ran";
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
        if (written && ran == 0u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine never ran\n", key / 1024u, z);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

static const struct interpretive_code *const codes[] = {
    &ORBCHG2_CODE,
    &P20LEM1_CODE,
    &P20LEMC3_CODE,
    &UPPSV_CODE,
    &ORBCHG3_CODE,
    &R22LEM3_CODE,
    &R22LEM96_CODE,
    &R22LEM96_WAIT_CODE,
    &R22LEM7_CODE,
    &R21LEM1_CODE,
    &R21LEM3_CODE,
    &R21LEM12_CODE,
    &R21LEM12_FIRST_PASS_CODE,
    &R21LEM7_CODE,
    &R21LEM8_CODE,
    &R61C_L03_CODE,
    &R61LEM1_AUTO_CODE,
    &R61C_L05_CODE,
};
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0, readable_basic = 0, readable_pairs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < LM_P20_P25_VECTORS; ++i)
        if (lm_p20_p25_cases[i].cm == 0u) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&lm_p20_p25_basic, &faithful_lm_p20_p25_basic, "basic");
    failures +=
        tables_differ(&lm_p20_p25_interpretive, &faithful_lm_p20_p25_interpretive, "interpretive");
    failures += parts_unwritten(&faithful_lm_p20_p25_basic, false);
    failures += parts_unwritten(&faithful_lm_p20_p25_interpretive, true);
    if (failures) {
        printf("p20-p25-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&lm_p20_p25_basic, &faithful_lm_p20_p25_basic, 0, &runs, &readable_basic);
    failures += keys(&lm_p20_p25_interpretive, &faithful_lm_p20_p25_interpretive, 1, &runs,
                     &readable_pairs);
    printf("p20-p25-cases: %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each (the others from %u), %u run by the faithful routines "
           "(%u of them a store past the E-bank window the readable binding refuses), "
           "%u failures\n",
           words, readable_basic + readable_pairs,
           lm_p20_p25_basic.count + lm_p20_p25_interpretive.count, readable_basic, readable_pairs,
           CASES, OTHER_CASES, runs, spills, failures);
    return failures != 0;
}
