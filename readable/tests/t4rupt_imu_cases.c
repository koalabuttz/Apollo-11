/* SPDX-License-Identifier: MIT */
/* The readable IMU monitor against the faithful port's, case by case (make
 * t4rupt-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * t4rupt_imu_routines.c) and the faithful port's under the same name with
 * faithful_ in front (the Makefile renames it). Each of the 22 entries
 * runs on 65,536 random states: random memory, mostly in bank 06; channel
 * 30 and 33 mostly near IMODES30 and IMODES33, so a few bits change at a
 * time; IMODES30's flags random; CDUZ anywhere, now and then +0, -0 or
 * near the 70 and 85 degree limits; RUPTREG1 and RUPTREG2 mostly a bit
 * scan as the scans leave it, now and then not; ITEMP6 mostly one of
 * SETISSW's returns in this file. Where either port runs, the other must
 * too, with the same memory, io record and channels, word for word; where
 * the readable one refuses, it must leave everything as it found it. The
 * faithful routines are certified against the original program
 * (c64-port/tests/test_lm_t4rupt_imu.c and test_cm_t4rupt_imu.c), as the
 * readable ones are by make t4rupt-cases' oracle step. */
#include <stdio.h>
#include <string.h>
#if LUMINARY099
#include "lm_t4rupt_imu.h"
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION      "LM"
#define TABLE        lm_t4rupt_imu_routines
#define FAITHFUL     faithful_lm_t4rupt_imu_routines
#define IMODES30_AT  01302u
#define IMODES33_AT  01303u
#define NXTIFAIL_AT  02225u /* SETISSW's returns in the file */
#define ISSUP_BACK   02337u
#define PIPFAIL_BACK 02656u
#else
#include "cm_t4rupt_imu.h"
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION      "CM"
#define TABLE        cm_t4rupt_imu_routines
#define FAITHFUL     faithful_cm_t4rupt_imu_routines
#define IMODES30_AT  01320u
#define IMODES33_AT  01321u
#define NXTIFAIL_AT  02227u
#define ISSUP_BACK   02332u
#define PIPFAIL_BACK 02640u
#endif
extern const agc_routine_table FAITHFUL;

#define CASES 65536u

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

/* A 16-bit register: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    return rnd() % 16u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

/* A few random bits of mask. */
static uint16_t few(uint16_t mask)
{
    return (uint16_t)(rnd() & rnd() & rnd() & mask);
}

/* A bit scan as the scans leave it: RUPTREG1 the position just handled
 * (-1 before the first, -0 or +0 for the first), RUPTREG2 the changes
 * left, each at a TCF of IFAILJMP (0-3, 5) or C33JMP (0-2). */
static void scan(apollo_overlay_state *s, int channel30)
{
    static const int from30[6] = {-1, 0, 1, 2, 3, 5}, from33[3] = {-1, 0, 1};
    int handled = channel30 ? from30[rnd() % 6u] : from33[rnd() % 3u], slot;
    uint16_t changes = 0;

    for (slot = handled + 1; slot <= 5; ++slot)
        if ((channel30 ? slot != 4 : slot <= 2) && rnd() % 2u)
            changes |= (uint16_t)(1u << (14 - (slot - handled)));
    s->words[070] = handled < 0                  ? 077776u
                    : handled == 0 && rnd() % 2u ? 077777u
                                                 : (uint16_t)handled;
    s->words[071] = changes;
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t z)
{
    unsigned i;
    uint16_t eb = (uint16_t)(rnd() % 8u), x;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[2] = (uint16_t)(rnd() % 2u ? 06000u + rnd() % 0700u : sign_extended(rnd()));
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = rnd() % 16u ? 014000u : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = z;
    s->words[6] = (uint16_t)(s->words[4] | eb);
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    /* CDUZ: anywhere, +0, -0, or near 70 or 85 degrees either way. */
    x = (uint16_t)(rnd() % 8u);
    if (x == 0u) s->words[034] = 0u;
    if (x == 1u) s->words[034] = 077777u;
    if (x == 2u) s->words[034] = (uint16_t)(014344u + rnd() % 8u - 4u);
    if (x == 3u) s->words[034] = (uint16_t)(017000u + rnd() % 01000u);
    if (x == 4u) s->words[034] = (uint16_t)(~(014344u + rnd() % 8u - 4u) & 077777u);
    /* IMODES30 and IMODES33: mostly near their flight values. */
    if (rnd() % 2u) {
        s->words[IMODES30_AT] = (uint16_t)(037411u ^ few(077777u));
        s->words[IMODES33_AT] = (uint16_t)(016000u ^ few(077777u));
    }
    /* RUPTREG1, RUPTREG2 and ITEMP6. */
    if (rnd() % 8u) scan(s, rnd() % 2u);
    x = (uint16_t)(rnd() % 6u);
    s->words[066] = x == 0u   ? NXTIFAIL_AT
                    : x == 1u ? ISSUP_BACK
                    : x == 2u ? PIPFAIL_BACK
                    : x == 3u ? (uint16_t)(04000u + rnd() % 04000u)
                    : x == 4u ? (uint16_t)(rnd() & 077777u)
                              : (uint16_t)(02000u + rnd() % 02000u);
    if (x == 0u) scan(s, 1);
    if (x == 2u) scan(s, 0);
    io->superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    hw->in_interrupt = (uint16_t)(rnd() % 4u ? 1u : rnd() % 3u);
    hw->channel11 = (uint16_t)(rnd() & 077777u);
    hw->channel12 = (uint16_t)(rnd() & 077777u);
    hw->channel14 = (uint16_t)(rnd() & 077777u);
    if (rnd() % 2u) hw->channel14 &= (uint16_t)~077000u;
    hw->channel30 =
        rnd() % 4u ? (uint16_t)(s->words[IMODES30_AT] ^ few(077777u)) : (uint16_t)(rnd() & 077777u);
    hw->channel33 =
        rnd() % 4u ? (uint16_t)(s->words[IMODES33_AT] ^ few(077777u)) : (uint16_t)(rnd() & 077777u);
}

int main(void)
{
    unsigned i, n, failures = 0;
    const agc_routine_table *readable = &TABLE, *faithful = &FAITHFUL;

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
        printf("%s t4rupt_imu %05o: %u cases, %u run by the faithful routine, %u failures\n",
               MISSION, z, CASES, ran, bad);
    }
    printf("t4rupt-imu-cases %s: %u failures\n", MISSION, failures);
    return failures != 0;
}
