/* SPDX-License-Identifier: MIT */
/* The readable restarts against the faithful port's, case by case (make
 * restart-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * restart_routines.c) and the faithful port's under the same name with
 * faithful_ in front (the Makefile renames it). Every entry runs on random
 * machine states from the same start through both. Where the faithful
 * routine runs, the readable one must too, and end with the same erasable
 * memory, registers and channels, word for word. Where the faithful one
 * refuses (a state it leaves to the original program), the readable one
 * may run; if it refuses too, it must leave the state as it found it. The
 * faithful routines are certified against the original program
 * (c64-port/tests/test_lm_restart_routines.c, test_cm_restart_routines.c),
 * and those tests run on the readable routines as well (make
 * restart-cases).
 *
 * The states are mostly ones the routines meet: GOPROG's banks, a group
 * 0-5, its phase one of the tables' or a variable one, the group's stored
 * 2CADR, priority or delta time, and for the CM's NEWPHASE a real TC
 * NEWPHASE site; with every condition broken now and then. L is
 * sign-corrected, as every instruction leaves it. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_transfer.h"
#include "agc_word.h"
#if LUMINARY099
#include "p66_bias.h"
#include "lm_restart_routines.h"
#define LM_RESTART_TABLE_PHASES
#include "lm_restart_table_data.inc"
typedef p66_bias_io mission_io;
#define MISSION     "LM"
#define TABLE       lm_restart_routines
#define FAITHFUL    faithful_lm_restart_routines
#define OPERANDS    lm_restart_operands
#define OPERAND_COUNT 2u
#define PHASES      lm_restart_table_phases
#define PHASE_COUNT LM_RESTART_TABLE_PHASE_COUNT
#define LONGBASE    01151u
#define PHSPART2_Z  LM_RS_PHSPART2
static agc_word sizetab(unsigned index)
{
    return lm_restart_table_data[index - LM_RESTART_TABLE_FIRST];
}
#else
#include "cm_native_image.h"
#include "cm_restart_routines.h"
#define CM_RESTART_TABLE
#define CM_RESTART_TABLE_PHASES
#include "cm_restart_table_data.inc"
typedef cm_native_io mission_io;
#define MISSION     "CM"
#define TABLE       cm_restart_routines
#define FAITHFUL    faithful_cm_restart_routines
#define OPERANDS    cm_restart_operands
#define OPERAND_COUNT CM_RESTART_OPERAND_COUNT
#define PHASES      cm_restart_table_phases
#define PHASE_COUNT CM_RESTART_TABLE_PHASE_COUNT
#define LONGBASE    01135u
#define PHSPART2_Z  CM_RR_PHSPART2
static agc_word sizetab(unsigned index)
{
    return cm_restart_table_words[index];
}
#endif
extern const agc_routine_table FAITHFUL;

#define CASES 200000u

/* Erasable addresses, the same in both missions. */
#define TEMPPHS  0154u
#define TEMP2G   0155u
#define POINTER  0156u
#define TEMPSWCH 0157u
#define GROUPIX  0161u
#define GOLOC    0705u
#define TIME2    024u
#define TIME1    025u
#define TBASE1   01053u
#define PHSPRDT1 01054u
#define PHSNAME1 01436u

static uint32_t seed = 12345u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* A register value: a sign-extended word, sometimes an overflow (A). */
static uint16_t reg(bool overflow)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (overflow && rnd() % 4u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return agc_extend(w);
}

/* A small time or address, or any word. */
static uint16_t some(uint16_t range)
{
    return (uint16_t)(rnd() % 4u ? rnd() % range : rnd() & 077777u);
}

static void keep_time_address(apollo_overlay_state *s);

static void state(apollo_overlay_state *s, mission_io *io, uint16_t z, bool erasable_key)
{
    static const agc_word variable[] = {00400, 00401, 00402, 00403, 00407, 01002, 01003,
                                        01402, 01403, 01005, 00404};
    static const agc_word services[2][4] = {
#if LUMINARY099
        {05105u, 05072u, 05203u, 05277u}, {04631u, 03763u, 03541u, 04631u}
#else
        {05147u, 05134u, 05245u, 05342u}, {04675u, 03755u, 03533u, 04675u}
#endif
    };
    unsigned i, g = rnd() % 6u, k = rnd() % PHASE_COUNT;
    agc_word phase;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg(true);
    s->words[1] = reg(false);
    s->words[2] = (uint16_t)(rnd() & 07777u);
    s->words[3] = 01400u;
    s->words[4] = 02000u;
    s->words[6] = 02003u;
    s->words[5] = z;
    io->interrupt_enabled = rnd() % 2u;
    /* The group and its phase: a table phase, a variable one, phase 1. */
    if (rnd() % 2u) {
        g = PHASES[k][0];
        phase = PHASES[k][1];
    } else
        phase = rnd() % 8u ? variable[rnd() % (sizeof variable / sizeof variable[0])] : 1u;
    s->words[GROUPIX] = (uint16_t)g;
    s->words[TEMPPHS] = phase;
    s->words[TEMP2G] = (uint16_t)(2u * g);
    s->words[TEMPSWCH] = services[1][rnd() % 2u];
    /* PHSPART2 follows an even table phase's first entry. */
    if (z == PHSPART2_Z)
        while (phase & 1u) {
            k = rnd() % PHASE_COUNT;
            g = PHASES[k][0];
            phase = PHASES[k][1];
            s->words[GROUPIX] = (uint16_t)g;
            s->words[TEMPPHS] = phase;
            s->words[TEMP2G] = (uint16_t)(2u * g);
        }
    /* Its pointer: that first entry. */
    s->words[POINTER] = (uint16_t)(sizetab(2u + 2u * g) + 3u * (phase & 0177u));
    /* The group's own words: a 2CADR in GOPROG's banks, a priority, a
     * delta time (direct, through an address, or now), its TBASE; and the
     * double times a LONGCALL reads. */
    s->words[PHSNAME1 + 2u * g] = (uint16_t)(01200u + rnd() % 0400u);
    s->words[PHSNAME1 + 1u + 2u * g] = (uint16_t)(rnd() % 4u ? 02003u : rnd() & 077777u);
    switch (rnd() % 5u) {
    case 0: s->words[PHSPRDT1 + 2u * g] = some(300u); break;
    case 1: s->words[PHSPRDT1 + 2u * g] = agc_negate((agc_word)(0100u + rnd() % 01277u)); break;
    case 2: s->words[PHSPRDT1 + 2u * g] = 077777u; break;
    case 3: s->words[PHSPRDT1 + 2u * g] = (uint16_t)(rnd() % 2u ? 010000u : 067777u); break;
    default: break;
    }
    s->words[TBASE1 + 2u * g] = some(040000u);
    s->words[TIME1] = some(040000u);
    s->words[TIME2] = some(4u);
    s->words[LONGBASE] = some(4u);
    s->words[LONGBASE + 1u] = some(040000u);
    /* The calls GOLOC -1 and GOLOC +2 hold. */
    s->words[GOLOC - 1u] = services[0][rnd() % 4u];
    s->words[GOLOC + 2u] = services[1][rnd() % 4u];
    (void)erasable_key;
#if COMANCHE055
    /* NEWPHASE: Q after a real TC NEWPHASE, in its bank. */
    if (z == CM_RR_NEWPHASE) {
        const agc_site_operand *site = &cm_restart_operands[2u + rnd() % CM_NEWPHASE_SITE_COUNT];
        s->words[4] = (uint16_t)((site->physical / 1024u) << 10);
        s->words[6] = (uint16_t)(s->words[4] | 3u);
        s->words[2] = (uint16_t)(02000u + site->physical % 1024u);
        s->words[0] = reg(true);
    }
#endif
    /* Now and then break one condition. */
    switch (rnd() % 32u) {
    case 0: s->words[6] = 02002u; break;
    case 1: s->words[GROUPIX] = (uint16_t)(6u + rnd() % 4u); break;
    case 2: s->words[TEMP2G] = (uint16_t)(rnd() % 16u); break;
    case 3: s->words[TEMPPHS] = (uint16_t)(rnd() & 077777u); break;
    case 5: io->interrupt_enabled = 2u; break;
    case 6: s->words[GOLOC - 1u] = (uint16_t)(rnd() & 07777u); break;
    case 7: s->words[GOLOC + 2u] = (uint16_t)(rnd() & 07777u); break;
    case 8: s->words[4] = 024000u; break;
    default: break;
    }
    keep_time_address(s);
}

/* A variable phase's delta time, read indirectly or for a LONGCALL, at an
 * address among the central registers or GOLOC (the faithful routine reads
 * them as they were before RESTARTS changed them), or a LONGCALL's double
 * time across 01377 and 01400 (it reads the second word unswitched). No
 * program stores such an address and the binding refuses it; such states
 * get a proper one. */
static void keep_time_address(apollo_overlay_state *s)
{
    agc_word phase = s->words[TEMPPHS], low = (agc_word)(phase & 7u);
    unsigned g = s->words[GROUPIX];
    agc_word *prdt;
    if (g >= 6u || !(phase & 01400u) || (phase & 01000u) || low == 2u) return;
    prdt = &s->words[PHSPRDT1 + 2u * g];
    if (low < 2u && (!(*prdt & 040000u) || *prdt == 077777u)) return;
    if (agc_negate(*prdt) < 010u || (low > 2u && agc_negate(*prdt) == 01377u) ||
        (agc_negate(*prdt) + 1u >= GOLOC - 1u && agc_negate(*prdt) <= GOLOC + 2u))
        *prdt = agc_negate((agc_word)(0100u + rnd() % 0400u));
}

static agc_routine find(const agc_routine_table *t, uint16_t key)
{
    uint16_t i;
    for (i = 0; i < t->count; ++i)
        if (t->entries[i].physical == key) return t->entries[i].call;
    return 0;
}

int main(void)
{
    agc_native_program program;
    unsigned i, checks = 0, failures = 0;

    memset(&program, 0, sizeof program);
    program.operands = OPERANDS;
    program.operand_count = OPERAND_COUNT;

    if (TABLE.count != FAITHFUL.count) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < FAITHFUL.count; ++i) {
        uint16_t key = FAITHFUL.entries[i].physical;
        bool erasable_key = key >= AGC_TRANSFER_ERASABLE_BASE;
        unsigned z = erasable_key  ? (unsigned)key - AGC_TRANSFER_ERASABLE_BASE
                     : key < 2048u ? 02000u + key % 1024u
                                   : 04000u + key - 2048u;
        agc_routine r = find(&TABLE, key), f = FAITHFUL.entries[i].call;
        unsigned n, ran = 0, bad = 0;
        if (!r) {
            printf("FAIL: no readable entry at %05o\n", z);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, a, b;
            mission_io io_start, ia, ib;
            enum exec_status sa, sb;
            bool good;
            state(&start, &io_start, (uint16_t)z, erasable_key);
            a = b = start;
            ia = ib = io_start;
            sa = f(&program, &a, &ia);
            sb = r(&program, &b, &ib);
            ++checks;
            ran += sa == EXEC_OK;
            if (sa == EXEC_OK)
                good = sb == EXEC_OK && !memcmp(&a, &b, sizeof a) && !memcmp(&ia, &ib, sizeof ia);
            else
                good = sb == EXEC_OK ||
                       (!memcmp(&start, &b, sizeof b) && !memcmp(&io_start, &ib, sizeof ib));
            if (!good && ++bad <= 3) {
                unsigned w;
                printf("FAIL %05o case %u: status %d, faithful %d\n", z, n, (int)sb, (int)sa);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (a.words[w] != b.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, b.words[w],
                               a.words[w], start.words[w]);
                if (memcmp(&ia, &ib, sizeof ia)) printf("  channels differ\n");
                if (getenv("RESTART_DEBUG")) {
                    unsigned g = start.words[GROUPIX];
                    printf("  pointer %o: %o %o %o\n", start.words[POINTER], sizetab(start.words[POINTER]), sizetab(start.words[POINTER] + 1u), sizetab(start.words[POINTER] + 2u));
                    printf("  phase %o group %u prdt %o name %o,%o tbase %o time %o,%o longbase %o,%o\n",
                           start.words[TEMPPHS], g, start.words[PHSPRDT1 + 2u * g],
                           start.words[PHSNAME1 + 2u * g], start.words[PHSNAME1 + 1u + 2u * g],
                           start.words[TBASE1 + 2u * g], start.words[TIME2], start.words[TIME1],
                           start.words[LONGBASE], start.words[LONGBASE + 1u]);
                }
            }
        }
        failures += bad;
        printf("%s %05o: %u cases, %u run, %u failures\n", MISSION, z, CASES, ran, bad);
    }
    printf("restart-cases %s: %u checks, %u failures\n", MISSION, checks, failures);
    return failures != 0;
}
