/* SPDX-License-Identifier: MIT */
/* The readable radar checks and GPMATRIX against the faithful port's, case
 * by case (make t4rupt-cases), the LM's only.
 *
 * Built with the readable table (src/binding/t4rupt_rr_routines.c) and the
 * faithful port's under the same name with faithful_ in front (the
 * Makefile renames it). Each of the 12 entries runs on 65,536 random
 * states: random memory, mostly in bank 06 and EBANK 6; RADMODES and
 * channels 30 and 33 mostly agreeing but for a bit or two; the flag words
 * random; the gimbal angles anywhere; A now and then an overflow (refused
 * after SPSIN and SPCOS). Where either port runs, the other must too, with
 * the same memory, io record and channels, word for word; where the
 * readable one refuses, it must leave everything as it found it. The
 * faithful routines are certified against the original program
 * (c64-port/tests/test_lm_t4rupt_rr.c), as the readable ones are by make
 * t4rupt-cases' oracle step. */
#include <stdio.h>
#include <string.h>
#include "lm_t4rupt_rr.h"
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION     "LM"
#define RADMODES_AT 0110u
extern const agc_routine_table faithful_lm_t4rupt_rr_routines;

#define CASES 65536u

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

/* A 16-bit register: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    return rnd() % 8u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t z)
{
    unsigned i;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[2] = sign_extended(rnd());
    s->words[3] = (uint16_t)((rnd() % 4u ? 6u : rnd() % 8u) << 8);
    s->words[4] = rnd() % 16u ? 014000u : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = z;
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    io->superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    hw->in_interrupt = (uint16_t)(rnd() % 4u ? 1u : rnd() % 3u);
    hw->channel12 = (uint16_t)(rnd() & 077777u);
    /* Channels 30 and 33 mostly as RADMODES last sampled them. */
    hw->channel30 = rnd() % 4u ? (uint16_t)(s->words[RADMODES_AT] ^ (rnd() & rnd() & 0102u))
                               : (uint16_t)(rnd() & 077777u);
    hw->channel33 = rnd() % 4u ? (uint16_t)(s->words[RADMODES_AT] ^ (rnd() & rnd() & 0102u))
                               : (uint16_t)(rnd() & 077777u);
}

int main(void)
{
    unsigned i, n, failures = 0;
    const agc_routine_table *readable = &lm_t4rupt_rr_routines,
                            *faithful = &faithful_lm_t4rupt_rr_routines;

    if (readable->count != faithful->count) {
        printf("FAIL: %u entries, faithful %u\n", readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        agc_routine f = faithful->entries[i].call, r = readable->entries[i].call;
        unsigned ran = 0, bad = 0;

        if (readable->entries[i].physical != key) {
            printf("FAIL entry %u at key %u, faithful %u\n", i, readable->entries[i].physical, key);
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
            if (n % 64u == 63u) fp.hardware = rp.hardware = 0; /* no hardware record */
            sf = f(&fp, &fa, &fio);
            sr = r(&rp, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                                  memcmp(&rhw, &hw, sizeof rhw)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
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
                if (memcmp(&fhw, &rhw, sizeof fhw))
                    printf("  channels 11 %05o/%05o 12 %05o/%05o 14 %05o/%05o 33 %05o/%05o\n",
                           rhw.channel11, fhw.channel11, rhw.channel12, fhw.channel12,
                           rhw.channel14, fhw.channel14, rhw.channel33, fhw.channel33);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %05o: the faithful routine never ran\n", z);
            ++bad;
        }
        failures += bad;
        printf("%s t4rupt_rr %05o: %u cases, %u run by the faithful routine, %u failures\n",
               MISSION, z, CASES, ran, bad);
    }
    printf("t4rupt-rr-cases %s: %u failures\n", MISSION, failures);
    return failures != 0;
}
