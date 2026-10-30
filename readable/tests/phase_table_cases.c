/* SPDX-License-Identifier: MIT */
/* The readable phase table maintenance against the faithful port's, case
 * by case (make phase-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * phase_table_tables.c) and the faithful port's under the same name with
 * faithful_ in front (the Makefile renames it). Every entry runs on random
 * machine states from the same start through both. Where the faithful
 * routine runs, the readable one must too, and end with the same erasable
 * memory, registers and channels, word for word. Where the faithful one
 * refuses (a state it leaves to the original program), the readable one
 * may run; if it refuses too, it must leave the state as it found it. The
 * faithful routines are certified against the original program
 * (c64-port/tests/test_lm_phase_table.c, test_cm_phase_table.c), and that
 * test runs on the readable routines as well (make phase-cases).
 *
 * The states are mostly ones the routines meet in flight: Q after a real
 * TC PHASCHNG (or NEWMODEX, CHECKMM, 2PHSCHNG) site, the phase words
 * there or ones like them, restart groups 0-7; with every condition broken
 * now and then, and overflows in A. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#if LUMINARY099
#include "p66_bias.h"
#include "lm_phase_table.h"
#define LM_PHASE_SITE_LIST
#include "lm_phase_sites.inc"
typedef p66_bias_io mission_io;
#define MISSION    "LM"
#define TABLE      lm_phase_table_routines
#define FAITHFUL   faithful_lm_phase_table_routines
#define SITES      lm_phase_sites
#define SITE_WORDS LM_PHASE_SITE_WORDS
#define SITE_LIST  lm_phase_site_list
#define SITE_COUNT LM_PHASE_SITE_COUNT
#else
#include "cm_native_image.h"
#include "cm_phase_table.h"
#define CM_PHASE_SITE_LIST
#include "cm_phase_sites.inc"
typedef cm_native_io mission_io;
#define MISSION    "CM"
#define TABLE      cm_phase_table_routines
#define FAITHFUL   faithful_cm_phase_table_routines
#define SITES      cm_phase_sites
#define SITE_WORDS CM_PHASE_SITE_WORDS
#define SITE_LIST  cm_phase_site_list
#define SITE_COUNT CM_PHASE_SITE_COUNT
#endif
extern const agc_routine_table FAITHFUL;

#define CASES 200000u

/* Erasable addresses (the same in both missions where they are named
 * here; the rest come from the listing through the readable build). */
#define TEMPG    061u
#define TEMPSW   065u
#define TEMPSW2  066u
#define TEMPG2   071u
#define RUPTREG1 070u

static uint32_t seed = 12345u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* A register value: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w & 040000u) << 1));
}

/* A word the routines read as a phase word: one after a real site, or
 * random. */
static uint16_t phase_word(void)
{
    agc_word w = SITES[rnd() % SITE_WORDS].word;
    return (uint16_t)(rnd() % 4u ? w & 077777u : rnd() & 077777u);
}

static void set_bank(apollo_overlay_state *s, mission_io *io, unsigned bank)
{
    io->superbank = 0;
    if (bank >= 040u) {
        bank -= 010u;
        io->superbank = 0100u;
    } else if (bank >= 030u && rnd() % 2u)
        io->superbank = 0060u;
    s->words[4] = (uint16_t)(bank << 10);
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
}

static void state(apollo_overlay_state *s, mission_io *io, uint16_t z, bool in_bank10)
{
    unsigned i, site = rnd() % SITE_COUNT;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[3] = (uint16_t)((rnd() % 8u) << 8);
    io->interrupt_enabled = rnd() % 2u;
    /* Q: after a real site, or anywhere in fixed or erasable memory. */
    set_bank(s, io, SITE_LIST[site].physical / 1024u);
    s->words[2] = (uint16_t)(02000u + SITE_LIST[site].physical % 1024u);
    if (rnd() % 8u == 0u) s->words[2] = (uint16_t)(0100u + rnd() % 07700u);
    if (rnd() % 16u == 0u) s->words[2] = (uint16_t)(s->words[2] + 1u + rnd() % 3u);
    /* The phase change's own words: groups 0-7 (doubled), the switches it
     * sets, and the caller's BBANK in L as PHASJUMP's DTCB leaves it. */
    s->words[TEMPG] = (uint16_t)(2u * (rnd() % 8u));
    s->words[TEMPG2] = (uint16_t)(2u * (rnd() % 8u));
    s->words[TEMPSW] = (uint16_t)(rnd() % 4u ? phase_word() : (rnd() % 4u) << 13);
    switch (rnd() % 4u) {
    case 0: s->words[TEMPSW2] = 1u; break;
    case 1: s->words[TEMPSW2] = 0u; break;
    case 2: s->words[TEMPSW2] = 040000u; break;
    default: break;
    }
    if (in_bank10) {
        /* The returns these routines DTCB to are addresses: the AGC's Z
         * has 12 bits (the faithful port would keep 15 in it). */
        s->words[RUPTREG1] = (uint16_t)(rnd() % 010000u);
        s->words[2] &= 07777u;
        s->words[1] = s->words[6];
        s->words[4] = 020000u;
        s->words[6] = (uint16_t)(020000u | (s->words[3] >> 8));
    }
    /* The phase word in A (PHSCHNGA) or the new major mode (NEWMODEA). */
    if (rnd() % 2u) s->words[0] = (uint16_t)(rnd() % 2u ? phase_word() : rnd() & 0177u);
    s->words[5] = z;
    /* Now and then break one condition. */
    switch (rnd() % 32u) {
    case 0: s->words[7] = 1; break;
    case 1: io->superbank = 1; break;
    case 2: io->interrupt_enabled = 2; break;
    case 3: s->words[4] ^= 024000u; break;
    case 4: s->words[2] = (uint16_t)(rnd() % 0100u); break;
    case 5:
        if (!in_bank10) s->words[2] = (uint16_t)(rnd() | (rnd() << 1));
        break;
    case 6: s->words[0] = (uint16_t)(040000u + rnd() % 040000u); break;
    default: break;
    }
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
    program.operands = SITES;
    program.operand_count = SITE_WORDS;

    if (TABLE.count != FAITHFUL.count) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < FAITHFUL.count; ++i) {
        uint16_t key = FAITHFUL.entries[i].physical;
        bool in_bank10 = key >= 010u * 1024u;
        uint16_t z = (uint16_t)(in_bank10 ? 02000u + key % 1024u : 04000u + key - 2048u);
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
            state(&start, &io_start, z, in_bank10);
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
            }
        }
        failures += bad;
        printf("%s %05o: %u cases, %u run, %u failures\n", MISSION, z, CASES, ran, bad);
    }
    printf("phase-cases %s: %u checks, %u failures\n", MISSION, checks, failures);
    return failures != 0;
}
