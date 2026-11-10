/* SPDX-License-Identifier: MIT */
/* The readable AOSTASK_AND_AOSJOB against the faithful port's, key by key
 * (make aostask-cases), the LM's only.
 *
 * Built with the readable tables (src/binding/aostask_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them), and the ropes loaded (tests/with_ropes.c: the
 * readable routines read the coefficient tables, GIMBLBTS, the jet failure
 * masks and DVOVSUB's divisors from the rope). First the keys, which must
 * be the faithful table's. Then 65,536 runs, each key in turn, from random
 * states: random memory, FB bank 20 and mostly EBANK 6 with the bank
 * registers agreeing, A now and then an overflow, L and Q any word;
 * DAPBOOLS with its docked, drift and USEQRJTS bits either way, FLGWRD10's
 * APSFLBIT either way, T5ADR mostly PAXIS; the masses, ABDELV, DB, the jet
 * accelerations and the offsets now real-sized, now +0, -0, small or any
 * word; INGTS, ALLOWGTS, FLATEMP, FLAT and the deadbands the same; channel
 * 12 and CH5MASK random; and now and then a bank, a ZERO, a latch or a word
 * the faithful routines refuse, and the hardware record missing. DOCKTEST
 * gets DOCKTEMP as 1/ACCS leaves it (the CSMDOCKD bit or +0), and INVERT a
 * return outside the file's code (1/ACCSET to 03777) and not 3: the
 * readable routines refuse the rest (DVOVSUB entered by DOCKTEST's CCS, a
 * return into the middle of the file), which the program never does.
 * Where either port runs, the other must too, with the same memory, io
 * record and hardware record, word for word; where the readable one
 * refuses, it must leave everything as it found it. */
#include <stdio.h>
#include <string.h>
#include "lm_aostask.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#define LUMINARY099 1
#include "../src/agc/lm_names.inc"

extern const agc_routine_table faithful_lm_aostask_basic;

#define RUNS 65536u
#define KEYS 11u

/* An erasable address's word under EBANK 6. */
#define E6(a) ((a) < 01400u ? (a) : 03000u + ((a) - 01400u))

static uint32_t seed = 2447u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 3) % n);
}

static uint16_t sign_extended(uint16_t v)
{
    return (uint16_t)((v & 077777u) | ((v & 040000u) << 1));
}

/* +0, -0, a small number of either sign, or any word. */
static uint16_t stirred(void)
{
    uint16_t v = (uint16_t)pick(64u);

    switch (pick(5u)) {
    case 0: return 0u;
    case 1: return 077777u;
    case 2: return v;
    case 3: return (uint16_t)(~v & 077777u);
    default: return (uint16_t)(rnd() & 077777u);
    }
}

/* A positive word of about the size given, or now and then a stirred one. */
static uint16_t sized(uint16_t about)
{
    if (pick(4u) == 0u) return stirred();
    return (uint16_t)(about / 2u + pick(about + 1u));
}

static void set_bit(uint16_t *w, uint16_t bit)
{
    if (pick(2u))
        *w |= bit;
    else
        *w &= (uint16_t)~bit;
}

/* The program's passive fixed data: the rope. */
static bool rope(const agc_native_program *program, uint16_t physical, agc_word *word)
{
    (void)program;
    return apollo_rope_read(0u, physical, word);
}

static void state(apollo_overlay_state *s, lm_channel_io *io, agc_hardware *hw, unsigned key)
{
    uint16_t *w = s->words, eb = pick(16u) ? 6u : (uint16_t)pick(8u), q;
    unsigned i;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) w[i] = (uint16_t)(rnd() & 077777u);
    w[0] = pick(8u) ? sign_extended(stirred()) : (uint16_t)(rnd() | 0100000u);
    w[1] = sign_extended(rnd());
    w[2] = sign_extended(rnd());
    w[3] = (uint16_t)(eb << 8);
    w[4] = pick(64u) ? 040000u : (uint16_t)(pick(040u) << 10);
    w[5] = faithful_lm_aostask_basic.entries[key].physical % 1024u + 02000u;
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    if (pick(128u) == 0u) w[7] = 1u;
    if (pick(256u) == 0u) w[010u + pick(APOLLO_SHARED_WORDS - 010u)] = 0100000u;
    io->io.superbank = (uint16_t)(pick(8u) * 020u);
    io->io.interrupt_enabled = (uint16_t)pick(2u);
    if (pick(128u) == 0u) io->io.interrupt_enabled = 2u;
    hw->channel12 = (uint16_t)(rnd() & 077777u);

    w[K_DAPBOOLS] = (uint16_t)(rnd() & 077777u);
    set_bit(&w[K_DAPBOOLS], KW_CSMDOCKD);
    set_bit(&w[K_DAPBOOLS], KW_DRIFTBIT);
    set_bit(&w[K_DAPBOOLS], KW_USEQRJTS);
    if (pick(3u)) w[K_DAPBOOLS] &= (uint16_t)~KW_USEQRJTS;
    set_bit(&w[K_FLGWRD10], KW_APSFLBIT);
    if (pick(4u)) w[E6(K_T5ADR)] = K_PAXIS;
    w[E6(K_MASS)] = sized(04000u);
    w[E6(K_MASS) + 1u] = (uint16_t)(rnd() & 077777u);
    w[E6(K_CSMMASS)] = sized(07000u);
    w[E6(K_LEMMASS)] = sized(04000u);
    w[E6(K_HIASCENT)] = sized(01200u);
    w[E6(K_ABDELV)] = sized(02000u);
    w[E6(K_DB)] = sized(01000u);
    w[E6(K_L_PVT_CG)] = sized(010000u);
    for (i = 0; i < 5u; ++i) w[E6(K_1JACC) + i] = sized(01000u);
    for (i = 0; i < 4u; ++i) w[E6(K_AOSQ) + i] = pick(2u) ? stirred() : sized(0400u);
    w[E6(K_INGTS)] = stirred();
    w[E6(K_ALLOWGTS)] = pick(2u) ? (uint16_t)pick(2u) : stirred();
    w[E6(K_FLAT)] = pick(2u) ? stirred() : sized(0400u);
    w[K_FLATEMP] = pick(2u) ? stirred() : sized(0400u);
    for (i = 0; i < 3u; ++i) w[K_DBVAL2 + i] = stirred();
    w[E6(K_CH5MASK)] = pick(2u) ? 0u : (uint16_t)(rnd() & 0377u);
    w[K_DOCKTEMP] = stirred();
    if (key == 6u) w[K_DOCKTEMP] = pick(2u) ? KW_CSMDOCKD : 0u;
    if (key == 7u) {
        w[0] = sign_extended(pick(2u) ? stirred() : sized(0400u));
        w[1] = sign_extended(pick(2u) ? stirred() : sized(0400u));
    }
    if (key == 10u) {
        do q = pick(8u) ? (uint16_t)pick(010000u) : (uint16_t)(rnd() & 077777u);
        while (q == 3u || (q >= K_1_ACCSET && q < 04000u));
        w[2] = sign_extended(q);
        if (pick(16u) == 0u) w[2] ^= 040000u; /* the same return, as an overflow */
    }
}

static unsigned same_keys(void)
{
    unsigned i;

    if (lm_aostask_basic.count != faithful_lm_aostask_basic.count ||
        lm_aostask_basic.count != KEYS || lm_aostask_interpretive.count != 0u) {
        printf("FAIL: the keys are not the faithful table's\n");
        return 1u;
    }
    for (i = 0; i < lm_aostask_basic.count; ++i)
        if (lm_aostask_basic.entries[i].physical != faithful_lm_aostask_basic.entries[i].physical) {
            printf("FAIL: the keys are not the faithful table's\n");
            return 1u;
        }
    return 0u;
}

int port_main(void)
{
    unsigned n, ran[KEYS] = {0}, failures = same_keys();

    if (failures) return 1;
    for (n = 0; n < RUNS; ++n) {
        apollo_overlay_state start, fa, ra;
        lm_channel_io io, fio, rio;
        agc_hardware hw, fhw, rhw;
        agc_native_program fp, rp;
        enum exec_status sf, sr;
        const char *why = 0;
        unsigned key = n % KEYS;
        int no_io = pick(64u) == 0u, no_hardware = pick(16u) == 0u;

        state(&start, &io, &hw, key);
        fa = ra = start;
        fio = rio = io;
        fhw = rhw = hw;
        memset(&fp, 0, sizeof fp);
        memset(&rp, 0, sizeof rp);
        fp.hardware = no_hardware ? 0 : &fhw;
        rp.hardware = no_hardware ? 0 : &rhw;
        fp.fixed_data = rp.fixed_data = rope;
        sf = faithful_lm_aostask_basic.entries[key].call(&fp, &fa, no_io ? 0 : &fio);
        sr = lm_aostask_basic.entries[key].call(&rp, &ra, no_io ? 0 : &rio);
        ran[key] += sf == EXEC_OK;
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
            why = "different io or hardware record";
        if (why && ++failures <= 5u) {
            unsigned w;
            printf("FAIL case %u key %u: %s (status %d, faithful %d)\n", n, key, why, (int)sr,
                   (int)sf);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    for (n = 0; n < KEYS; ++n)
        if (ran[n] < RUNS / KEYS / 4u) {
            printf("FAIL: key %u: the faithful routine ran in %u runs\n", n, ran[n]);
            ++failures;
        }
    printf("aostask-cases: %u runs, faithful runs", RUNS);
    for (n = 0; n < KEYS; ++n) printf(" %u", ran[n]);
    printf(", %u failures\n", failures);
    return failures != 0u;
}
