/* SPDX-License-Identifier: MIT */
/* The readable keyboard and uplink interrupts against the faithful
 * port's, case by case (make keyrupt-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * keyrupt_routines.c) and the faithful port's under the same name with
 * faithful_ in front (the Makefile renames it). Each entry runs on 65,536
 * random states: random memory, with the bank registers coherent; uplink
 * words a third of them triple codes (a sixth of those ERROR RESET);
 * UPLOCKFL either way; LOCCTR mostly at a core set; and some states
 * outside the routines' domain (an overflow in A or Q, FBANK elsewhere,
 * outside the interrupt). Where the faithful routine runs, the readable
 * one must too, with the same memory, io record and channels, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it. The faithful routines are certified against the original
 * program (c64-port/tests/test_keyrupt_uprupt.c), as the readable ones
 * are by make keyrupt-cases' first step. */
#include <stdio.h>
#include <string.h>
#include "keyrupt_uprupt.h"
#if LUMINARY099
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION  "LM"
#define TABLE    lm_keyrupt_routines
#define FAITHFUL faithful_lm_keyrupt_routines
#define FB       010000u
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION  "CM"
#define TABLE    cm_keyrupt_routines
#define FAITHFUL faithful_cm_keyrupt_routines
#define FB       016000u
#endif

extern const agc_routine_table FAITHFUL;

#define CASES 65536u

static uint32_t seed = 141421u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* An uplink word: a third of them the code three times (low and high five
 * bits the code, the middle five its complement), a sixth of those ERROR
 * RESET (22); the rest arbitrary. */
static uint16_t uplink(void)
{
    unsigned kind = rnd() % 6u;
    uint16_t code = kind == 0u ? 022u : (uint16_t)(rnd() & 037u);
    if (kind < 2u) return (uint16_t)(code << 10 | (~code & 037u) << 5 | code);
    return (uint16_t)(rnd() & 077777u);
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t z)
{
    unsigned i;
    uint16_t eb = (uint16_t)(rnd() % 8u);

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = rnd() % 16u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
    s->words[1] = sign_extended(rnd());
    s->words[2] = rnd() % 16u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = rnd() % 32u ? FB : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = z;
    s->words[6] = (uint16_t)(s->words[4] | eb);
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    s->words[045] = uplink();                                                   /* INLINK */
    s->words[064] = (uint16_t)(rnd() % 8u ? 12u * (rnd() % 8u) : rnd() % 100u); /* LOCCTR */
    io->superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    hw->in_interrupt = (uint16_t)(rnd() % 32u ? 1u : rnd() % 3u);
    hw->channel11 = (uint16_t)(rnd() & 077777u);
    hw->channel15 = (uint16_t)(rnd() & 077777u);
}

int main(void)
{
    unsigned i, n, checks = 0, failures = 0;

    if (TABLE.count != FAITHFUL.count) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < FAITHFUL.count; ++i) {
        uint16_t key = FAITHFUL.entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        agc_routine f = FAITHFUL.entries[i].call, r = TABLE.entries[i].call;
        unsigned ran = 0, bad = 0;
        if (TABLE.entries[i].physical != key) {
            printf("FAIL: entry %u at key %u, faithful %u\n", i, TABLE.entries[i].physical, key);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_hardware hw, fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, &hw, z);
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hw;
            memset(&fp, 0, sizeof fp);
            memset(&rp, 0, sizeof rp);
            fp.hardware = &fhw;
            rp.hardware = &rhw;
            sf = f(&fp, &fa, &fio);
            sr = r(&rp, &ra, &rio);
            ++checks;
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                                  memcmp(&rhw, &hw, sizeof rhw)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK &&
                     (memcmp(&fio, &rio, sizeof fio) || memcmp(&fhw, &rhw, sizeof fhw)))
                why = "different io or channels";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %05o case %u: %s\n", z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        failures += bad;
        printf("%s %05o: %u cases, %u run by the faithful routine, %u failures\n", MISSION, z,
               CASES, ran, bad);
    }
    printf("keyrupt-cases %s: %u checks, %u failures\n", MISSION, checks, failures);
    return failures != 0;
}
