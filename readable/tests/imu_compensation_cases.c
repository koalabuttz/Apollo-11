/* SPDX-License-Identifier: MIT */
/* The readable IMU compensation against the faithful port's, key by key
 * (make imu-compensation-cases), one mission per build (LUMINARY099=1 or
 * COMANCHE055=1).
 *
 * Built with the readable table (src/binding/imu_compensation_routines.c)
 * and the faithful port's under the same name with faithful_ in front
 * (the Makefile renames it). First the keys, which must be the faithful
 * table's. Then each key runs from 4,096 random states of the kind the
 * flight program enters it with: bank 06 with the bank registers
 * agreeing, ZERO +0, every erasable word an ordinary one, E-bank 3 (GCOMP's)
 * mostly, otherwise any. Most states are stirred further: GCOMPSW and
 * FLAGWRD2 in each of CCS's four classes; small coefficients, PIPA counts
 * and gyro commands (GCOMP), so that commands of two pulses or less, and
 * more, both come up; TEM1 as NBDONLY leaves it (+0 or BIT8), now and then
 * anything; and the times NBD2 takes the difference of equal (the CM's -0,
 * "ELAPSED TIME = 0"). Each runs once on each side, as the program's
 * trampoline would call the key's routine. Where either port runs, the
 * other must too, with the same memory and io record, word for word;
 * where the readable one refuses, it must leave everything as it found
 * it; and the faithful routine must run from some state of every key. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#include "overlay_runtime.h"

#if LUMINARY099
#include "p66_bias.h"
#include "../src/agc/lm_names.inc"
typedef p66_bias_io mission_io;
#define TABLE          lm_imu_compensation_routines
#define FAITHFUL_TABLE faithful_lm_imu_compensation_routines
#define PIPTIME_LOW    (K_PIPTIME + 1u) /* AD PIPTIME +1 */
#define NAME           "LM"
#else
#include "cm_native_image.h"
#include "../src/agc/cm_names.inc"
typedef cm_native_io mission_io;
#define TABLE          cm_imu_compensation_routines
#define FAITHFUL_TABLE faithful_cm_imu_compensation_routines
#define PIPTIME_LOW    (K_PIPTIME1 + 1u) /* AD PIPTIME1 +1 */
#define NAME           "CM"
#endif

extern const agc_routine_table TABLE;
extern const agc_routine_table FAITHFUL_TABLE;

#define CASES 4096u

static uint32_t seed = 314159u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

/* A word of CCS's class k: positive, +0, negative, -0. */
static uint16_t of_class(unsigned k)
{
    uint16_t v = (uint16_t)(rnd() % 037776u + 1u);
    return k == 0u ? v : k == 1u ? 0u : k == 2u ? (uint16_t)(~v & 077777u) : 077777u;
}

/* A small signed word: +0 to 7 or -0 to -7. */
static uint16_t small(void)
{
    uint16_t v = (uint16_t)(rnd() % 8u);
    return rnd() % 2u ? (uint16_t)(~v & 077777u) : v;
}

/* One state at the key: a random one, and for most n more. */
static void state(apollo_overlay_state *s, mission_io *io, uint16_t key, unsigned n)
{
    uint16_t z = (uint16_t)(02000u + key % 1024u), eb = 3u, banks[2];
    unsigned i, b;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 3u; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = sign_extended(rnd());
    s->words[1] = sign_extended(rnd());
    s->words[2] = (uint16_t)(rnd() & 077777u);
    s->words[7] = 0u;
    s->words[025] = (uint16_t)(rnd() & 037777u); /* TIME1: centiseconds, 14 bits */
    if (n % 8u == 7u) eb = (uint16_t)(rnd() % 8u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & 0160u);
    if (n % 4u != 0u) {
        /* The compensation's words in E-bank 3, where 1/PIPA and GCOMPZER
         * set it, and in the bank the state has. */
        banks[0] = 3u;
        banks[1] = eb;
        for (b = 0; b < 2u; ++b) {
            unsigned base = banks[b] * 0400u;
            if (n % 4u == 1u) /* NBD..ADSRA, PIPABIAS, PIPASCF small */
                for (i = 052u; i <= 070u; ++i) s->words[base + i] = small();
            if (n % 4u != 3u) /* GCOMP: small whole pulses */
                for (i = 0; i < 6u; i += 2u) s->words[base + 071u + i] = small();
            s->words[base + 077u] = of_class((n / 4u) % 4u); /* GCOMPSW */
        }
        if (n % 4u == 1u)
            for (i = 0; i < 6u; ++i) s->words[K_DELVX + i] = (uint16_t)(rnd() % 4u);
        s->words[K_FLAGWRD2] = of_class((n / 16u) % 4u);
        s->words[K_TEM1] = rnd() % 2u ? 0200u : 0u;
        if (rnd() % 16u == 0u) s->words[K_TEM1] = (uint16_t)(rnd() & 077777u);
        /* Equal times: NBDONLY's TIME1 and 1/PIPADT; LASTBIAS's PIPTIME
         * +1 and 1/PIPADT; or +0 against -0. */
        switch (rnd() % 8u) {
        case 0: s->words[K_1_PIPADT] = s->words[025]; break;
        case 1: s->words[K_1_PIPADT] = s->words[PIPTIME_LOW]; break;
        case 2:
            s->words[025] = 0u;
            s->words[PIPTIME_LOW] = 0u;
            s->words[K_1_PIPADT] = 077777u;
            break;
        default: break;
        }
    }
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = 014000u;
    s->words[5] = z;
    s->words[6] = (uint16_t)(014000u | eb);
}

int main(void)
{
    unsigned i, n, failures = 0;
    const agc_routine_table *readable = &TABLE, *faithful = &FAITHFUL_TABLE;

    if (readable->count != faithful->count) {
        printf("FAIL: %u keys, faithful %u\n", readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i)
        if (readable->entries[i].physical != faithful->entries[i].physical) {
            printf("FAIL: key %u is %u, faithful %u\n", i, readable->entries[i].physical,
                   faithful->entries[i].physical);
            ++failures;
        }
    if (failures) {
        printf("imu-compensation-cases (" NAME "): %u failures\n", failures);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            mission_io io, fio, rio;
            agc_hardware hw;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, n);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            memset(&hw, 0, sizeof hw);
            p.hardware = n % 2u ? &hw : 0;
            sf = faithful->entries[i].call(&p, &fa, &fio);
            sr = readable->entries[i].call(&p, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL 06,%04o case %u: %s\n", z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
                if (memcmp(&fio, &rio, sizeof fio))
                    printf("  interrupts %u, faithful %u\n", rio.interrupt_enabled,
                           fio.interrupt_enabled);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL 06,%04o: the faithful routine never ran\n", z);
            ++bad;
        }
        failures += bad;
    }
    printf("imu-compensation-cases (" NAME "): %u keys from %u states each, %u failures\n",
           faithful->count, CASES, failures);
    return failures != 0;
}
