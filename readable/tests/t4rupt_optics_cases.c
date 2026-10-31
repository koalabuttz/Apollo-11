/* SPDX-License-Identifier: MIT */
/* The readable optics routines against the faithful port's, case by case
 * (make t4rupt-cases), the CM's only.
 *
 * Built with the readable tables (src/binding/t4rupt_optics_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them). Each of the 8 entries runs on 65,536 random
 * states: random memory, mostly in the entry's bank (06 or 10); OPTMODES
 * and channels 30 and 33 mostly agreeing but for a bit or two; the switch
 * words (SWSAMPLE, DESOPMOD), OPTIND and the zeroing counts mostly the
 * values the routines set, now and then +0, -0 or anything; the CDU
 * angles and their desired angles anywhere, often near the 45 and 90
 * degree limits; channels 12 and 14 random. Where either port runs, the
 * other must too, with the same memory, io record and channels, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it. The faithful routines are certified against the original
 * program (c64-port/tests/test_cm_t4rupt_optics.c), as the readable ones
 * are by make t4rupt-cases' oracle step. */
#include <stdio.h>
#include <string.h>
#include "cm_t4rupt_optics.h"
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION "CM"
extern const agc_routine_table faithful_cm_t4rupt_optics_basic;

/* The erasable words the routines read (Comanche055's addresses). */
#define CDUT_AT     0035u
#define CDUS_AT     0036u
#define ITEMP1_AT   0061u
#define DESOPTT_AT  01160u
#define OPTIND_AT   01303u
#define COMMANDO_AT 01305u
#define ZONE_AT     01307u
#define SWSAMPLE_AT 01314u
#define DESOPMOD_AT 01315u
#define WTOPTION_AT 01316u
#define ZOPTCNT_AT  01317u
#define IMODES33_AT 01321u
#define OPTMODES_AT 01331u
#define LAMPS_AT    (01023u + 11u)

#define CASES 65536u

static uint32_t seed = 161803u;
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

/* A few random bits of mask. */
static uint16_t few(uint16_t mask)
{
    return (uint16_t)(rnd() & rnd() & rnd() & mask);
}

/* A small signed word, as the routines set them: +0, -0, -1, 1, or a few
 * either way; now and then anything. */
static uint16_t small(void)
{
    static const uint16_t set[] = {0u, 077777u, 077776u, 1u, 2u, 013u, 040u, 077775u};
    uint16_t x = (uint16_t)(rnd() % 10u);
    return x < 8u ? set[x] : (uint16_t)(rnd() & 077777u);
}

/* An angle: anywhere, or near +-45 or +-90 degrees (010000, 020000). */
static uint16_t angle(void)
{
    uint16_t x = (uint16_t)(rnd() % 6u), a;
    if (x < 2u) return (uint16_t)(rnd() & 077777u);
    a = (uint16_t)((x < 4u ? 010000u : 020000u) + rnd() % 8u - 4u);
    return rnd() % 2u ? a : (uint16_t)(~a & 077777u);
}

static void state(apollo_overlay_state *s, mission_io *io, agc_hardware *hw, uint16_t z,
                  uint16_t bank)
{
    unsigned i;
    uint16_t eb = (uint16_t)(rnd() % 8u);

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[2] = (uint16_t)(rnd() % 2u ? 02000u + rnd() % 02000u : sign_extended(rnd()));
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = rnd() % 16u ? bank : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = z;
    s->words[6] = (uint16_t)(s->words[4] | eb);
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    if (rnd() % 64u == 0u) s->words[3] = (uint16_t)(rnd() & 077777u);
    /* The optics' words. */
    s->words[OPTIND_AT] = small();
    s->words[SWSAMPLE_AT] = small();
    s->words[DESOPMOD_AT] = small();
    s->words[WTOPTION_AT] = small();
    s->words[ZOPTCNT_AT] = small();
    s->words[ZONE_AT] = rnd() % 2u ? small() : (uint16_t)(rnd() & 077777u);
    s->words[CDUT_AT] = angle();
    s->words[CDUS_AT] = angle();
    s->words[DESOPTT_AT] = rnd() % 2u ? angle() : (uint16_t)(s->words[CDUT_AT] ^ few(0377u));
    s->words[DESOPTT_AT + 1u] = rnd() % 2u ? angle() : (uint16_t)(s->words[CDUS_AT] ^ few(0377u));
    s->words[COMMANDO_AT] = angle();
    s->words[COMMANDO_AT + 1u] = angle();
    s->words[ITEMP1_AT] = small();
    s->words[OPTMODES_AT] = few(077777u);
    if (rnd() % 4u == 0u) s->words[IMODES33_AT] = few(1u);
    if (rnd() % 2u) s->words[LAMPS_AT] = few(040200u);
    io->superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->interrupt_enabled = (uint16_t)(rnd() % 2u);
    if (rnd() % 64u == 0u) io->interrupt_enabled = 2u;
    io->channel12 = (uint16_t)(rnd() & 077777u);
    io->channel14 = (uint16_t)(rnd() & 077777u);
    hw->in_interrupt = (uint16_t)(rnd() % 4u ? 1u : rnd() % 3u);
    /* Channels 30 and 33 mostly as OPTMODES last sampled them. */
    hw->channel30 =
        rnd() % 4u ? (uint16_t)(s->words[OPTMODES_AT] ^ few(0100u)) : (uint16_t)(rnd() & 077777u);
    hw->channel33 =
        rnd() % 4u ? (uint16_t)(s->words[OPTMODES_AT] ^ few(030u)) : (uint16_t)(rnd() & 077777u);
}

int main(void)
{
    unsigned i, n, failures = 0;
    const agc_routine_table *readable = &cm_t4rupt_optics_basic,
                            *faithful = &faithful_cm_t4rupt_optics_basic;

    if (readable->count != faithful->count) {
        printf("FAIL: %u entries, faithful %u\n", readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u),
                 bank = (uint16_t)((key / 1024u) << 10);
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

            state(&start, &io, &hw, z, bank);
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
                if (memcmp(&fio, &rio, sizeof fio))
                    printf("  channels 12 %05o/%05o 14 %05o/%05o\n", rio.channel12, fio.channel12,
                           rio.channel14, fio.channel14);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %05o: the faithful routine never ran\n", z);
            ++bad;
        }
        failures += bad;
        printf("%s t4rupt_optics %05o: %u cases, %u run by the faithful routine, %u failures\n",
               MISSION, z, CASES, ran, bad);
    }
    printf("t4rupt-optics-cases %s: %u failures\n", MISSION, failures);
    return failures != 0;
}
