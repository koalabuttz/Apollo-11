/* SPDX-License-Identifier: MIT */
/* The readable alarms and aborts against the faithful port's, case by case
 * (make alarm-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * alarm_routines.c) and the faithful port's under the same name with
 * faithful_ in front (the Makefile renames it). Every entry runs on random
 * machine states from the same start through both. Where the faithful
 * routine runs, the readable one must too, with the same erasable memory,
 * registers, channels and interrupt state, word for word. Where the
 * faithful routine refuses (leaves the state to the original program), the
 * readable one may run it; where the readable one refuses, it must leave
 * the state as it found it. The faithful routines are certified against
 * the original program (c64-port/tests/test_lm_alarm.c, test_cm_alarm.c),
 * as the readable ones are by make alarm-cases' first step.
 *
 * The program's operands are the mission's alarm sites (the code after
 * every TC ALARM and the rest), one now and then made negative or taken
 * away. Q is mostly at a site, A mostly an alarm code, each FAILREG word
 * +0, -0, positive or negative, and ALMCADR and BUF2 mostly addresses;
 * now and then a condition is broken.
 *
 * Two kinds of state no program makes, where the faithful routine departs
 * from the AGC (yaAGC) and the readable one refuses, are not counted:
 *   - the LM's FLAGS returning by TC Q with an overflow in Q: the faithful
 *     routine goes to the overflow-corrected address, where the AGC runs
 *     the 16-bit Q as some other instruction;
 *   - the CM's BAILOUT and POODOO after VAC5STOR with a negative word in
 *     ALMCADR: the faithful routine indexes with it as if positive. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#if LUMINARY099
#include "lm_alarm.h"
#define LM_ALARM_SITE_LIST
#include "../src/lm_alarm_sites.inc"
typedef p66_bias_io mission_io;
#define MISSION       "LM"
#define TABLE         lm_alarm_routines
#define FAITHFUL      faithful_lm_alarm_routines
#define OPERANDS      lm_alarm_operands
#define OPERAND_COUNT LM_ALARM_OPERAND_COUNT
#define SITES         lm_alarm_site_list
#define SITE_COUNT    LM_ALARM_SITE_COUNT
#else
#include "cm_alarm.h"
#define CM_ALARM_SITE_LIST
#include "../src/cm_alarm_operands.inc"
typedef cm_native_io mission_io;
#define MISSION       "CM"
#define TABLE         cm_alarm_routines
#define FAITHFUL      faithful_cm_alarm_routines
#define OPERANDS      cm_alarm_operands
#define OPERAND_COUNT CM_ALARM_OPERAND_COUNT
#define SITES         cm_alarm_site_list
#define SITE_COUNT    CM_ALARM_SITE_COUNT
#endif

extern const agc_routine_table FAITHFUL;

#define CASES   40000u
#define ALMCADR 01363u
#define FAILREG 0375u
#define BUF2    0133u

static uint32_t seed = 4711u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static agc_site_operand operands[OPERAND_COUNT];

/* A register value: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 8u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w & 040000u) << 1));
}

/* A FAILREG word: +0, -0, positive or negative. */
static uint16_t failreg(void)
{
    uint16_t v = (uint16_t)(1u + rnd() % 037776u);
    switch (rnd() % 4u) {
    case 0: return 0;
    case 1: return 077777u;
    case 2: return v;
    default: return (uint16_t)(v | 040000u);
    }
}

/* Q (or ALMCADR) at a random alarm site, with the banks that reach it. */
static uint16_t at_site(apollo_overlay_state *s, mission_io *io)
{
    uint16_t physical = SITES[rnd() % SITE_COUNT].physical, bank = (uint16_t)(physical / 1024u);
    if (bank == 2u || bank == 3u) return (uint16_t)(bank * 02000u + physical % 1024u);
    if (bank >= 040u) {
        bank = (uint16_t)(bank - 010u);
        io->superbank = 0100u;
    } else if (bank >= 030u)
        io->superbank = 0u;
    s->words[4] = (uint16_t)(bank << 10);
    return (uint16_t)(02000u + physical % 1024u);
}

static uint16_t address(void)
{
    switch (rnd() % 6u) {
    case 0: return (uint16_t)(010u + rnd() % 01770u); /* erasable */
    case 1: return 07777u;
    case 2: return (uint16_t)(rnd() | (rnd() << 1));
    default: return (uint16_t)(02000u + rnd() % 06000u);
    }
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t key)
{
    unsigned i;
    uint16_t z, fixed_bank = 0xffffu;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    for (i = 0; i < 3u; ++i) s->words[FAILREG + i] = failreg();
    s->words[0] = rnd() % 2u ? reg() : (uint16_t)(rnd() % 040000u);
    s->words[1] = reg();
    s->words[3] = (uint16_t)((rnd() % 8u) << 8);
    s->words[4] = (uint16_t)((rnd() % 040u) << 10);
    io->superbank = (uint16_t)(rnd() & 0160u);
    io->interrupt_enabled = rnd() % 2u;
    hw->in_interrupt = rnd() % 2u;
    hw->channel11 = (agc_word)(rnd() & 077777u);
    if (key >= AGC_TRANSFER_ERASABLE_BASE)
        z = (uint16_t)(key - AGC_TRANSFER_ERASABLE_BASE);
    else if (key >= 2048u && key < 4096u)
        z = (uint16_t)(04000u + key - 2048u);
    else {
        fixed_bank = (uint16_t)(key / 1024u);
        z = (uint16_t)(02000u + key % 1024u);
    }
    s->words[2] = rnd() % 4u ? at_site(s, io) : address();
    if (rnd() % 2u)
        s->words[ALMCADR] = (uint16_t)((rnd() % 2u ? at_site(s, io) : address()) & 077777u);
    if (rnd() % 2u) s->words[BUF2] = (uint16_t)(02000u + rnd() % 06000u);
    if (fixed_bank != 0xffffu) s->words[4] = (uint16_t)(fixed_bank << 10);
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
    s->words[5] = z;
    switch (rnd() % 32u) {
    case 0: s->words[7] = 1; break;
    case 1: io->superbank = 1; break;
    case 2: io->interrupt_enabled = 2; break;
    case 3: s->words[2] = (uint16_t)(rnd() | (rnd() << 1)); break;
    case 4: s->words[0] = (uint16_t)(rnd() | (rnd() << 1)); break;
    default: break;
    }
}

/* An entry's place, as the listing shows it. */
static const char *where(uint16_t key)
{
    static char text[24];
    if (key >= AGC_TRANSFER_ERASABLE_BASE)
        sprintf(text, "E %04o", (unsigned)(key - AGC_TRANSFER_ERASABLE_BASE));
    else if (key >= 2048u && key < 4096u)
        sprintf(text, "%05o", (unsigned)(04000u + key - 2048u));
    else
        sprintf(text, "%02o,%04o", (unsigned)(key / 1024u), (unsigned)(02000u + key % 1024u));
    return text;
}

/* A state of the two kinds above. */
static bool faithful_departs(uint16_t key, const apollo_overlay_state *s)
{
#if LUMINARY099
    return key == 013u * 1024u + (LM_AL_FLAGS - 02000u) &&
           ((s->words[2] >> 14) & 1u) != ((s->words[2] >> 15) & 1u);
#else
    return (key == 2048u + (CM_AL_BAILOUT_RET - 04000u) ||
            key == 2048u + (CM_AL_POODOO_RET - 04000u)) &&
           (s->words[ALMCADR] & 040000u);
#endif
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
    agc_hardware hw_a, hw_b;
    unsigned i, checks = 0, failures = 0, departs = 0;

    if (TABLE.count != FAITHFUL.count) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    memset(&program, 0, sizeof program);
    program.operands = operands;

    for (i = 0; i < FAITHFUL.count; ++i) {
        uint16_t key = FAITHFUL.entries[i].physical;
        agc_routine r = find(&TABLE, key), f = FAITHFUL.entries[i].call;
        unsigned n, ran = 0, refused = 0, bad = 0;
        if (!r) {
            printf("FAIL: no readable entry at key %u\n", key);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state a, b, start;
            mission_io ia, ib, istart;
            enum exec_status sa, sb;
            unsigned o;
            const char *why = 0;
            /* The operands: now and then one site's code negative or gone. */
            memcpy(operands, OPERANDS, sizeof operands);
            program.operand_count = OPERAND_COUNT;
            o = rnd() % OPERAND_COUNT;
            if (rnd() % 16u == 0u)
                operands[o].word = (agc_word)((040000u | rnd()) & 077777u);
            else if (rnd() % 16u == 0u) {
                memmove(&operands[o], &operands[o + 1u],
                        (OPERAND_COUNT - o - 1u) * sizeof operands[0]);
                program.operand_count = OPERAND_COUNT - 1u;
            }
            state(&a, &ia, &hw_a, key);
            program.hardware = rnd() % 64u ? &hw_a : 0;
            b = a;
            ib = ia;
            hw_b = hw_a;
            start = a;
            istart = ia;
            sa = f(&program, &a, &ia);
            if (program.hardware) program.hardware = &hw_b;
            sb = r(&program, &b, &ib);
            ++checks;
            if (sa == EXEC_OK) ++ran;
            if (sb != EXEC_OK) ++refused;
            if (sb != EXEC_OK) {
                if (memcmp(&b, &start, sizeof b) || memcmp(&ib, &istart, sizeof ib))
                    why = "readable refusal changed the state";
                else if (sa == EXEC_OK && faithful_departs(key, &start))
                    ++departs;
                else if (sa == EXEC_OK)
                    why = "readable refused, faithful ran";
            } else if (sa == EXEC_OK && (memcmp(&a, &b, sizeof a) || memcmp(&ia, &ib, sizeof ia) ||
                                         memcmp(&hw_a, &hw_b, sizeof hw_a)))
                why = "different";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL key %u case %u: %s (status %d, faithful %d)\n", key, n, why, (int)sb,
                       (int)sa);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (a.words[w] != b.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, b.words[w],
                               a.words[w], start.words[w]);
                if (ia.superbank != ib.superbank || ia.interrupt_enabled != ib.interrupt_enabled)
                    printf("  channels differ\n");
                if (memcmp(&hw_a, &hw_b, sizeof hw_a)) printf("  hardware differs\n");
                printf("  start Q %06o A %06o L %06o FB %06o EB %06o SB %03o IE %o ALMCADR "
                       "%05o,%05o\n",
                       start.words[2], start.words[0], start.words[1], start.words[4],
                       start.words[3], istart.superbank, istart.interrupt_enabled,
                       start.words[ALMCADR], start.words[ALMCADR + 1u]);
            }
        }
        failures += bad;
        printf("%s alarm entry %s: %u cases, %u run by the faithful routine, %u refused by the "
               "readable one, %u failures\n",
               MISSION, where(key), CASES, ran, refused, bad);
    }
    printf("alarm-cases %s: %u checks (%u where the faithful routine departs from the AGC), %u "
           "failures\n",
           MISSION, checks, departs, failures);
    return failures != 0;
}
