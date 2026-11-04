/* SPDX-License-Identifier: MIT */
/* The readable P32-P33_P72-P73 against the faithful port's, key by key
 * (make cm-p32-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_p32_routines.c) and the
 * faithful port's (c64-port/src/cm_p32_p33.c) under the same names with
 * faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/p32_p33_cm/),
 * written in the listing's notation, must be the original program's rope,
 * word for word. Then the keys, which must be the faithful tables', and
 * the keys the parts written so far hold must run readable code, the
 * others the faithful port's. Then the runs: each readable key from 4,096
 * states (each other key from 16), each one of the port's frozen test
 * states (c64-port/tests/cm_p32_p33_vectors.h) entered at the key, most
 * stirred further: MPAC and its mode, the push-down list and its pointer,
 * the index registers (X1 at ALMXIT's indexed load often round the seven
 * words of ALARM/TB), A (now and then with overflow), Q, the flag words
 * (often one bit changed), the vectors and times (any erasable bank's
 * words), NEWJOB, the overflow indicator, now and then any word but the
 * interpreter's own, the latches, or a state the routines refuse (another
 * work area, BBANK not the two banks, a word of 16 bits); a hardware
 * record mostly, which the routines do not use. Each runs once on each
 * side, as the program's trampoline would call the key's routine. Where
 * either port runs, the other must too, with the same memory, io record
 * and hardware record, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * from at least 1 in 16 of the states of every readable key. A key the
 * binding hands to the faithful port must give the faithful routine's
 * result, refusals and all. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_p32_p33_vectors.h"
#include "cm_interp.h"

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code P32STRT_CODE, ALMXITA_CODE, P32_P72A_CODE, VN0637_CODE,
    P32_P72E_CODE, P32_P72_N75_CODE, P33_P73A_CODE, P33_P73_ALARM_CODE, P33_P73C_CODE,
    P33_P73E_CODE, P33_P73_N75_CODE, DISDVLVC_CODE, DISDVLVC_EXIT_CODE, CSI_A_CODE, CSI_B2_CODE,
    CSI_B23_CODE, CSINEXT_CODE, NTP_2_CODE, CSINEXT1_CODE, CSINEXT2_CODE, K2_CODE, CSINEXT3_CODE,
    CSINEXT4_CODE;
static const struct interpretive_code *const codes[] = {
    &P32STRT_CODE,     &ALMXITA_CODE,  &P32_P72A_CODE,      &VN0637_CODE,   &P32_P72E_CODE,
    &P32_P72_N75_CODE, &P33_P73A_CODE, &P33_P73_ALARM_CODE, &P33_P73C_CODE, &P33_P73E_CODE,
    &P33_P73_N75_CODE, &DISDVLVC_CODE, &DISDVLVC_EXIT_CODE, &CSI_A_CODE,    &CSI_B2_CODE,
    &CSI_B23_CODE,     &CSINEXT_CODE,  &NTP_2_CODE,         &CSINEXT1_CODE, &CSINEXT2_CODE,
    &K2_CODE,          &CSINEXT3_CODE, &CSINEXT4_CODE,
};
extern const agc_routine_table cm_p32_p33_basic, cm_p32_p33_interpretive;
extern const agc_routine_table faithful_cm_p32_p33_basic, faithful_cm_p32_p33_interpretive;
bool cm_p32_p33_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
#define NEWOPS_AT   06037u
#define X1_AT       0046u /* X1, X2, S1 and S2, in the work area */
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
#define QPRET_AT    052u                    /* the work area's QPRET */
#define NORMEX_AT   01340u                  /* DISDVLVC's return, a CADR */
#define SUBEXIT_AT  02370u                  /* E4,1770: SUBEXIT, a return */
#define ALMXIT_X1   (04u * 1024u + 01320u)  /* 4,3320: ALMXIT's SLOAD* ALARM/TB -1,1 */
#define PMINE_X1_A  (031u * 1024u + 01610u) /* 31,3610 and 31,3616: CSINEXT4's DSU* PMINE -2,1 */
#define PMINE_X1_B  (031u * 1024u + 01616u)

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 3337u;
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

/* The keys the parts written so far hold: part 1, the programs (banks 4
 * and 14, bank 12 to P33/P73D's end, and bank 37 to DISDVLVC's end); parts
 * 2 and 3, CSI/A to SCNDSOL (banks 6, 7, 10, 11, 12, 22, 23 and 24, and
 * bank 31 to SCNDSOL's end). */
static bool in_written_part(uint16_t key)
{
    unsigned bank = key / 1024u, z = 02000u + key % 1024u;

    return bank == 04u || bank == 06u || bank == 07u || bank == 010u || bank == 011u ||
           bank == 012u || bank == 014u || bank == 022u || bank == 023u || bank == 024u ||
           (bank == 031u && z < 03662u) || (bank == 037u && z < 02645u);
}

static unsigned parts_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (in_written_part(key) != cm_p32_p33_readable(key, order_pair)) {
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
    return rnd() % 4u ? any_word() : edges[rnd() % COUNT(edges)];
}

/* The frozen cases. */
static unsigned mission_cases[CM_P32_P33_VECTORS], mission_count;

/* A return address (a TC's), or any word: a TC through an erasable word
 * takes only an address 010-07777. */
static uint16_t return_word(void)
{
    return rnd() % 4u ? (uint16_t)(010u + rnd() % 07770u) : rnd() % 2u ? any_word() : rnd() % 010u;
}

/* A CADR in fixed memory, as STQ leaves one: mostly the word now if it
 * is one, else any. A GOTO through an erasable word takes the CADR there. */
static uint16_t fixed_cadr(uint16_t now)
{
    return (now & 074000u) ? now : (uint16_t)(04000u + rnd() % 074000u);
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hardware, uint16_t key,
                  int interpretive, unsigned n)
{
    const irc_case *c = &cm_p32_p33_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hardware, 0, sizeof *hardware);
    irc_build(s, c);
    if (irc_ch_on) irc_cm_io(io, hardware, 0);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    /* The file's banks 31, 35 and 37 want the superbank bit clear. */
    if (rnd() % 4u) io->superbank &= (uint16_t)~0100u;
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
        /* The vectors and times: any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A (now and then with overflow), Q. */
        k = rnd() % 4u;
        w[0] = k < 2u ? any_word() : k == 2u ? rnd() : (uint16_t)(rnd() % 2u ? 040000u : 0137777u);
        if (rnd() % 2u) w[2] = return_word();
        /* The index registers. */
        if (rnd() % 2u) w[(fixloc + X1_AT + rnd() % 4u) & 03777u] = any_word();
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
        eb = (uint16_t)(w[3] >> 8);
        /* SUBEXIT, a return: a TC's address, or any word. */
        if (rnd() % 2u) w[SUBEXIT_AT] = return_word();
    }
    /* Now and then a work area the routines do not take: a NOVAC job's
     * (FAKEPRET, MPAC -36D), one a word off a VAC area, or none. */
    if (n % 4u != 0u && rnd() % 64u == 0u) {
        static const uint16_t near[] = {0400u, 0402u, 0430u, 0662u, 0735u};
        k = rnd() % 3u;
        fixloc = w[FIXLOC_AT] = k == 0u   ? (uint16_t)(MPAC_AT - 044u)
                                : k == 1u ? near[rnd() % COUNT(near)]
                                          : (uint16_t)(rnd() % 02000u);
    }
    /* The push-down list in the work area. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    /* At ALMXIT's indexed load, often X1 -1 to -7 (-CSIALRM, the alarm
     * number), the loads from ALARM/TB's seven words, and now and then -11
     * to +3, round their ends. */
    if (key == ALMXIT_X1) {
        k = rnd() % 4u;
        if (k < 2u)
            w[(fixloc + X1_AT) & 03777u] = (uint16_t)(077777u - (1u + rnd() % 7u));
        else if (k == 2u) {
            k = rnd() % 15u;
            w[(fixloc + X1_AT) & 03777u] = (uint16_t)(k < 11u ? 077777u - (k + 1u) : k - 11u);
        }
    }
    /* At CSINEXT4's indexed loads, often X1 -2 or -10 (RTX1 at the Earth
     * or the Moon), the loads from PMINE and PMINM, and now and then -12
     * to +2, round them. */
    if (key == PMINE_X1_A || key == PMINE_X1_B) {
        k = rnd() % 4u;
        if (k < 2u)
            w[(fixloc + X1_AT) & 03777u] = rnd() % 2u ? 077775u : 077765u;
        else if (k == 2u) {
            k = rnd() % 15u;
            w[(fixloc + X1_AT) & 03777u] = (uint16_t)(k < 12u ? 077777u - (k + 1u) : k - 12u);
        }
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* NORMEX, DISDVLVC's return, which its GOTO takes, and the work
     * area's QPRET, which RVQ takes: CADRs in fixed memory as the STQs
     * leave them. (One in erasable memory could send the faithful GOTO
     * round a cycle of CADRs, which it ends by exiting.) At an order pair
     * SUBEXIT too. */
    w[NORMEX_AT] = fixed_cadr(w[NORMEX_AT]);
    if (interpretive) w[SUBEXIT_AT] = fixed_cadr(w[SUBEXIT_AT]);
    w[(fixloc + QPRET_AT) & 03777u] = fixed_cadr(w[(fixloc + QPRET_AT) & 03777u]);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* The interrupt latch either way now and then. */
    if (n % 4u != 0u && rnd() % 8u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 2u);
}

/* A pass that starts at a store code (a CHANG2 resume) and would store
 * past the end of the E-bank window: the AGC writes into fixed memory and
 * reads it back into A and L, which the readable machine does not model,
 * so the binding refuses it, as the faithful port's other interpretive
 * routines do (cmi_store_spills); the faithful routines of this file run
 * it. */
static unsigned spills;

static bool faithful_spills(uint16_t key, const apollo_overlay_state *start)
{
    agc_word w = 0;
    bool spill = apollo_rope_read(1, key, &w) && !(w & 040000u) && cmi_store_spills(start, w);

    spills += spill;
    return spill;
}

/* An order pair in a bank below 30 with the superbank bit set: the
 * faithful pairs there take the bit either way, and read their constants
 * where the listing has them, where the AGC would read those in bank 35
 * from bank 45, which the rope lacks; the binding refuses it. */
static unsigned superbank_states;

static bool superbank_set_below_30(uint16_t key, int interpretive, const cm_native_io *io)
{
    bool set = interpretive && key / 1024u < 030u && (io->superbank & 0100u);

    superbank_states += set;
    return set;
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        bool written = cm_p32_p33_readable(key, interpretive != 0);
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
            /* A hardware record mostly, which the routines do not use. */
            if (rnd() % 4u) {
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = faithful->entries[i].call(&fp, &fa, &fio);
            sr = readable->entries[i].call(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (!written) { /* the faithful routine on both sides, refusals and all */
                if (sf != sr || memcmp(&fa, &ra, sizeof fa) || memcmp(&fio, &rio, sizeof fio) ||
                    memcmp(&fhw, &rhw, sizeof fhw))
                    why = "the table does not run the faithful routine";
            } else if (sr != EXEC_OK &&
                       (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                        memcmp(&rhw, &hardware, sizeof rhw)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = faithful_spills(key, &start) || superbank_set_below_30(key, interpretive, &io)
                          ? 0
                          : "readable refused, faithful ran";
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
        /* Each readable key's faithful routine must run in at least 1 of
         * 16 states, so that a change to the states cannot quietly leave a
         * key with a handful of live ones. (The states are tuned to a
         * part's keys when it is written.) */
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

int main(int argc, char **argv)
{
    unsigned i, words = 0, failures = 0, runs = 0, readable_basic = 0, readable_pairs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_P32_P33_VECTORS; ++i)
        if (cm_p32_p33_cases[i].cm) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&cm_p32_p33_basic, &faithful_cm_p32_p33_basic, "basic");
    failures +=
        tables_differ(&cm_p32_p33_interpretive, &faithful_cm_p32_p33_interpretive, "interpretive");
    failures += parts_unwritten(&faithful_cm_p32_p33_basic, false);
    failures += parts_unwritten(&faithful_cm_p32_p33_interpretive, true);
    if (failures || !mission_count) {
        printf("cm-p32-cases: %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&cm_p32_p33_basic, &faithful_cm_p32_p33_basic, 0, &runs, &readable_basic);
    failures += keys(&cm_p32_p33_interpretive, &faithful_cm_p32_p33_interpretive, 1, &runs,
                     &readable_pairs);
    printf("cm-p32-cases: %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each, the others from %u, %u run by the faithful routines, "
           "%u store spills and %u superbank states refused, %u failures\n",
           words, readable_basic + readable_pairs,
           cm_p32_p33_basic.count + cm_p32_p33_interpretive.count, readable_basic, readable_pairs,
           CASES, OTHER_CASES, runs, spills, superbank_states, failures);
    return failures != 0;
}
