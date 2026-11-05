/* SPDX-License-Identifier: MIT */
/* The readable LUNAR_LANDING_GUIDANCE_EQUATIONS against the faithful
 * port's, key by key (make llge-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/llge_routines.c) and the
 * faithful port's (c64-port/src/lm_llge.c) under the same names with
 * faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code (src/landing_guidance/),
 * written in the listing's notation, must be the LM's rope, word for word.
 * Then the keys, which must be the faithful tables', each running readable
 * code where the parts written so far cover it (WRITTEN). Then the runs:
 * each readable key from 4,096 states (any other from 16), each one of the
 * port's frozen test states (c64-port/tests/lm_llge_vectors.h) entered at
 * the key, most stirred further: MPAC and its mode, the push-down list and
 * its pointer, the index registers, A (now and then with overflow), Q,
 * the flag words (often one bit changed), the vectors and times (any
 * erasable bank's words), NEWJOB, the overflow indicator, now and then any
 * word but the interpreter's own, the latches, channels 13 and 31, no
 * hardware record, or a state the routines refuse (another work area,
 * BBANK not the two banks, a word of 16 bits); and the words part 1's
 * routines test: the redesignator's ELVIRA, ZERLINA and RODCOUNT (each
 * sign), ROOTPSRS's table pointers (mostly erasable addresses), its
 * counters (PWRCNT, MPAC +4, MODE) and its return (RETROOT), and WCHPHASE
 * (mostly -1 to 2). The words after ROOTPSRS's TC USPRCADRs are entered
 * at DANZIG with the EXIT in EDOP (now and then not). Each runs once on
 * each side, as the program's trampoline would call the key's routine.
 * Where either port runs, the other must too, with the same memory, io
 * record and channels, word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful routine must run
 * from at least 1 in 16 of the states of every readable key. A key the
 * binding hands to the faithful port must give the faithful routine's
 * result, refusals and all. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "lm_llge_vectors.h"

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code QUADGUID_CODE, CGCALC_CODE, EXGSUB_CODE, EXBRAK_CODE,
    EXNORM_CODE, GDUMP1_CODE, P65VERT_CODE, P67VERT_CODE, ITRPNT1_CODE, VHORCOMP_CODE;
static const struct interpretive_code *const codes[] = {
    &QUADGUID_CODE, &CGCALC_CODE,  &EXGSUB_CODE,  &EXBRAK_CODE,  &EXNORM_CODE,
    &GDUMP1_CODE,   &P65VERT_CODE, &P67VERT_CODE, &ITRPNT1_CODE, &VHORCOMP_CODE};
#define readable_basic_table        lm_llge_basic
#define readable_interpretive_table lm_llge_interpretive
#define faithful_basic              faithful_lm_llge_basic
#define faithful_interpretive       faithful_lm_llge_interpretive
#define frozen_cases                lm_llge_cases
#define FROZEN_CASES                LM_LLGE_VECTORS
#define MISSION_IS_CM               0u
#define ROPE                        0u
#define NEWOPS_AT                   06070u
#define DANZIG_AT                   06061u
/* The parts written so far: part 1, the redesignator trap (bank 11), the
 * rate of descent trap (bank 20), ROOTPSRS to INTPRETX (31,3553-3677, and
 * FASTCHNG, 31,3740) and P66 (32,3263-3531); part 2, QUADGUID to P67VERT
 * (31,3121-3552) and 1406ALM's return (31,3737). */
#define WRITTEN                                                                                    \
    {011u, 02275u, 02372u}, {020u, 02115u, 02122u}, {031u, 03121u, 03677u},                        \
        {031u, 03737u, 03740u}, {032u, 03263u, 03531u}
/* The words after ROOTPSRS's TC USPRCADRs, entered at DANZIG. */
#define AFTER_USPRCADR(key)                                                                        \
    ((key) == 031u * 1024u + 01575u || (key) == 031u * 1024u + 01642u ||                           \
     (key) == 031u * 1024u + 01647u)
extern const agc_routine_table readable_basic_table, readable_interpretive_table;
extern const agc_routine_table faithful_basic, faithful_interpretive;
bool llge_readable(uint16_t key, bool order_pair);

/* The addresses of the words the routines use. */
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
#define ELVIRA_AT   01265u
#define ZERLINA_AT  01264u
#define RODCOUNT_AT 01746u
#define RETROOT_AT  0132u
#define PWRPTR_AT   0117u
#define DERPTR_AT   0141u
#define PWRCNT_AT   0140u
#define WCHPHASE_AT 01351u
#define WCHPHOLD_AT 03621u

#define CASES       4096u
#define OTHER_CASES 16u

static uint32_t seed = 3435u;
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

/* Whether a key is in the parts written so far (its bank, and its
 * address in 02000-03777). */
static bool in_written_part(uint16_t key)
{
    static const struct {
        unsigned bank, first, last;
    } written[] = {WRITTEN};
    unsigned z = 02000u + key % 1024u, i;

    for (i = 0; i < sizeof written / sizeof written[0]; ++i)
        if (key / 1024u == written[i].bank && z >= written[i].first && z <= written[i].last)
            return true;
    return false;
}

static unsigned parts_unwritten(const agc_routine_table *table, bool order_pair)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (in_written_part(key) != llge_readable(key, order_pair)) {
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

/* A word of each sign now and then: +0, -0, 1, -1, or any. */
static uint16_t edge_or_any(void)
{
    static const uint16_t edges[] = {0u, 077777u, 1u, 077776u};
    return rnd() % 2u ? edges[rnd() % COUNT(edges)] : any_word();
}

/* The frozen cases. */
static unsigned mission_cases[FROZEN_CASES], mission_count;

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, lm_channel_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &frozen_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;
    uint16_t *w = s->words, fixloc;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
    /* Banks 31 and 32 want the superbank bit clear. */
    if (rnd() % 4u) io->io.superbank &= (uint16_t)~0100u;
    eb = (uint16_t)(w[3] >> 8);
    w[4] = (uint16_t)(bank << 10);
    if (interpretive && AFTER_USPRCADR(key)) {
        w[5] = DANZIG_AT; /* USPRCADR's EXIT pending, LOC the CADR */
        w[LOC_AT] = z;
        w[EDOP_AT] = rnd() % 8u ? 1u : (uint16_t)(rnd() % 4u);
    } else if (interpretive) {
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
        if (rnd() % 2u) w[2] = any_word();
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
        /* X1 as INTPRETX leaves it: minus the phase's target index (-0,
         * -034 or +1); else minus a small count. (Not a word that indexes
         * the guidance's tables out of their E-bank: past it the faithful
         * orders read the wrong words, which the readable interpreter
         * does not.) */
        if (rnd() % 8u) {
            static const uint16_t x1[] = {077777u, 077743u, 1u};
            w[(fixloc + X1_AT) & 03777u] = x1[rnd() % COUNT(x1)];
        } else {
            w[(fixloc + X1_AT) & 03777u] = (uint16_t)(077777u - rnd() % 0100u);
        }
        /* The redesignator's ELVIRA and ZERLINA (the last pass's bits and
         * the quiet passes left: each sign, or small) and RODCOUNT. */
        k = rnd() % 4u;
        if (k != 3u)
            w[ELVIRA_AT] = k == 0u ? 0u : k == 1u ? (uint16_t)(rnd() & 063u) : edge_or_any();
        k = rnd() % 4u;
        if (k != 3u)
            w[ZERLINA_AT] = k == 0u ? 077777u : k == 1u ? (uint16_t)(rnd() % 3u) : edge_or_any();
        if (rnd() % 2u) w[RODCOUNT_AT] = edge_or_any();
        /* ROOTPSRS: its table pointers (MPAC +3, PWRPTR, DERPTR), mostly
         * erasable addresses; N-1 and PWRCNT, small or each sign; MODE's
         * bit 4 now and then; the return in RETROOT, mostly a TC to fixed
         * memory. */
        k = rnd() % 8u;
        w[MPAC_AT + 3u] = k < 6u ? (uint16_t)(rnd() % 02000u) : any_word();
        w[PWRPTR_AT] = rnd() % 8u ? (uint16_t)(rnd() % 02000u) : any_word();
        /* (Not a DCA of 1777 through it: the faithful port's second word is
         * then A, the AGC's fixed memory's 2000. No table is there.) */
        if (w[PWRPTR_AT] == 01774u || w[PWRPTR_AT] == 01776u) w[PWRPTR_AT] = 0u;
        w[DERPTR_AT] = rnd() % 8u ? (uint16_t)(rnd() % 02000u) : any_word();
        if (rnd() % 2u) w[MPAC_AT + 4u] = rnd() % 2u ? (uint16_t)(rnd() % 6u) : edge_or_any();
        if (rnd() % 2u) w[PWRCNT_AT] = rnd() % 2u ? (uint16_t)(rnd() % 6u) : edge_or_any();
        if (bank == 031u && z >= 03553u && z <= 03677u && rnd() % 4u == 0u)
            w[MODE_AT] = (uint16_t)(rnd() % 2u ? 010u : rnd() % 020u); /* ROOTPSRS's flags */
        w[RETROOT_AT] = rnd() % 8u ? (uint16_t)(02000u + rnd() % 06000u) : any_word();
        /* WCHPHASE: -1 to 2, one past either end, -0, or any. */
        if (rnd() % 2u) {
            static const uint16_t phases[] = {077776u, 0u, 1u, 2u, 077775u, 3u, 077777u};
            k = rnd() % 8u;
            w[WCHPHASE_AT] = k < 7u ? phases[k] : any_word();
        }
        /* WCHPHOLD: the same, or the largest positive (+1 overflows). */
        k = rnd() % 12u;
        if (k < 6u) {
            static const uint16_t holds[] = {077776u, 0u, 1u, 2u, 037777u};
            w[WCHPHOLD_AT] = k < 5u ? holds[k] : any_word();
        }
        /* Channel 31: the hand controller's pulse bits and the attitude
         * hold bit (13), or any. */
        if (rnd() % 2u)
            io->in.channel31 = rnd() % 2u ? (uint16_t)((rnd() & 010063u) ^ 077777u) : any_word();
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
    }
    /* The push-down list in the work area. */
    if (w[PUSHLOC_AT] < fixloc || w[PUSHLOC_AT] > fixloc + 037u || n % 4u != 0u)
        w[PUSHLOC_AT] = (uint16_t)(fixloc + rnd() % 040u);
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
    w[BANKSET_AT] = rnd() % 32u ? w[6] : any_word();
    /* The interrupt latch either way now and then. */
    if (n % 4u != 0u && rnd() % 8u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 2u);
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *runs, unsigned *readable_keys)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        bool written = llge_readable(key, interpretive != 0);
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
            /* Channel 13 (RESETRPT's WOR): the trap bits (12-14) on or off,
             * or any; now and then no hardware record. */
            memset(&hardware, 0, sizeof hardware);
            hardware.channel13 = rnd() % 2u ? any_word() : (uint16_t)(rnd() % 8u << 11);
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
            if (!written) { /* the faithful routine on both sides, refusals and all */
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
        /* Each readable key's faithful routine must run in at least 1 of
         * 16 states, so that a change to the states cannot quietly leave a
         * key with a handful of live ones. */
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
    for (i = 0; i < FROZEN_CASES; ++i)
        if (frozen_cases[i].cm == MISSION_IS_CM) mission_cases[mission_count++] = i;
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&readable_basic_table, &faithful_basic, "basic");
    failures += tables_differ(&readable_interpretive_table, &faithful_interpretive, "interpretive");
    failures += parts_unwritten(&faithful_basic, false);
    failures += parts_unwritten(&faithful_interpretive, true);
    if (failures || !mission_count) {
        printf("llge-cases: %u failures\n", failures + !mission_count);
        return 1;
    }
    failures += keys(&readable_basic_table, &faithful_basic, 0, &runs, &readable_basic);
    failures +=
        keys(&readable_interpretive_table, &faithful_interpretive, 1, &runs, &readable_pairs);
    printf("llge-cases: %u words as the rope's, %u of %u keys readable (%u basic, %u order "
           "pairs) from %u states each (the others from %u), %u run by the faithful routines, "
           "%u failures\n",
           words, readable_basic + readable_pairs,
           readable_basic_table.count + readable_interpretive_table.count, readable_basic,
           readable_pairs, CASES, OTHER_CASES, runs, failures);
    return failures != 0;
}
