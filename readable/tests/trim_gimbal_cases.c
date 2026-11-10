/* SPDX-License-Identifier: MIT */
/* The readable TRIM_GIMBAL_CONTROL_SYSTEM against the faithful port's, key
 * by key (make trim-gimbal-cases), the LM's only.
 *
 * Built with the readable table (src/binding/trim_gimbal_routines.c) and
 * the faithful port's under the same name with faithful_ in front (the
 * Makefile renames it), and the ropes loaded (tests/with_ropes.c: the
 * readable routines read LIMITS, BIT15 on, GMBLBITA's neighbours and the
 * word DOSHIFT's indexed CA reads from the rope, and the faithful GTS that
 * word through the program's fixed_data, here the same rope). First the
 * keys, which must be the faithful table's. Then 65,536 runs, each key in
 * turn from the port's case image (c64-port/tests/lm_trim_gimbal_cases.c:
 * random memory, FB bank 21 or 16, EBANK 6, channel 12 random, the trim
 * gimbal inputs of random widths, and cohorts near the switching curve
 * where GTSQRT takes the zero root and DOSHIFT's CA reads another word),
 * most stirred further: A (now and then an overflow) and L; NEGUQ, NEGUR,
 * AOSQ, AOSR, ACCDOTQ, ACCDOTR, KQ, KR, EDOTQ, EDOTR, QDIFF and RDIFF +0,
 * -0, small or any word; SR any word; and now and then a bank, an EBANK, a
 * return in Q or RUPTREG2 the faithful routine refuses, and the io record
 * or the hardware record missing. Where either port runs, the other must
 * too, with the same memory, io record and channel 12, word for word;
 * where the readable one refuses, it must leave everything as it found
 * it. */
#include <stdio.h>
#include <string.h>
#include "lm_trim_gimbal_cases.h"
#include "rope_image.h"

extern const agc_routine_table faithful_lm_trim_gimbal_routines;

#define RUNS 65536u

/* EBANK 6 words, by their erasable addresses. */
#define EDOTQ    01436u
#define QDIFF    01450u
#define NEGUQ    01501u
#define KQ       01504u
#define ACCDOTQ  01507u
#define AOSQ     01537u
#define RUPTREG2 071u
#define E6(a)    (03000u + ((a) - 01400u))

static uint32_t seed = 3077u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
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

static uint16_t sign_extended(uint16_t v)
{
    return (uint16_t)(v & 040000u ? v | 0100000u : v);
}

/* The program's passive fixed data: the rope. */
static bool rope(const agc_native_program *program, uint16_t physical, agc_word *word)
{
    (void)program;
    return apollo_rope_read(0u, physical, word);
}

static void state(apollo_overlay_state *s, lm_flight_io *io, unsigned n)
{
    uint16_t *w = s->words;
    lm_tg_case c;
    unsigned i;

    memset(io, 0, sizeof *io);
    lm_tg_input(s, io, &c, (uint16_t)(n % LM_TG_CASES));
    if (n % 4u == 0u) return;
    if (pick(4u) == 0u) w[0] = pick(8u) ? sign_extended(stirred()) : (uint16_t)(rnd() | 0100000u);
    if (pick(4u) == 0u) w[1] = sign_extended(stirred());
    for (i = 0; i < 4u; ++i)
        if (pick(4u) == 0u) w[E6(AOSQ) + i] = stirred();
    for (i = 0; i < 3u; ++i) {
        if (pick(4u) == 0u) w[E6(NEGUQ) + i] = stirred();
        if (pick(4u) == 0u) w[E6(ACCDOTQ) + i] = stirred();
        if (pick(4u) == 0u) w[E6(KQ) + i] = stirred();
    }
    for (i = 0; i < 2u; ++i)
        if (pick(4u) == 0u) w[E6(EDOTQ) + i] = stirred();
    for (i = 0; i < 4u; ++i)
        if (pick(4u) == 0u) w[E6(QDIFF) + i] = stirred();
    if (pick(4u) == 0u) w[021] = (uint16_t)(rnd() & 077777u);
    if (pick(32u) == 0u)
        w[2] = pick(2u) ? (uint16_t)pick(02000u) : (uint16_t)(010000u + pick(010000u));
    if (pick(32u) == 0u) w[RUPTREG2] = (uint16_t)pick(02000u);
    if (pick(64u) == 0u) {
        w[3] = (uint16_t)(pick(8u) << 8);
        w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
    }
    if (pick(64u) == 0u) {
        w[4] = (uint16_t)(rnd() & 076000u);
        w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
    }
}

static unsigned same_keys(void)
{
    unsigned i;

    if (lm_trim_gimbal_routines.count != faithful_lm_trim_gimbal_routines.count) {
        printf("FAIL: the keys are not the faithful table's\n");
        return 1u;
    }
    for (i = 0; i < lm_trim_gimbal_routines.count; ++i)
        if (lm_trim_gimbal_routines.entries[i].physical !=
            faithful_lm_trim_gimbal_routines.entries[i].physical) {
            printf("FAIL: the keys are not the faithful table's\n");
            return 1u;
        }
    return 0u;
}

int port_main(void)
{
    unsigned n, ran[4] = {0, 0, 0, 0}, failures = same_keys();

    if (failures) return 1;
    for (n = 0; n < RUNS; ++n) {
        apollo_overlay_state start, fa, ra;
        lm_flight_io io, fio, rio;
        agc_native_program fp, rp;
        enum exec_status sf, sr;
        const char *why = 0;
        unsigned key, entry = n % 4u;
        int no_io = pick(64u) == 0u, no_hardware = pick(64u) == 0u;

        /* The port's entries are GTS, ACDT+C12, TIMEGMBL and DONEYET +6;
         * the table's keys are in address order, ACDT+C12 first. */
        key = entry == 0u ? 1u : entry == 1u ? 0u : entry;
        state(&start, &io, n);
        fa = ra = start;
        fio = rio = io;
        memset(&fp, 0, sizeof fp);
        memset(&rp, 0, sizeof rp);
        fp.hardware = no_hardware ? 0 : &fio.hw;
        rp.hardware = no_hardware ? 0 : &rio.hw;
        fp.fixed_data = rp.fixed_data = rope;
        sf = faithful_lm_trim_gimbal_routines.entries[key].call(&fp, &fa, no_io ? 0 : &fio);
        sr = lm_trim_gimbal_routines.entries[key].call(&rp, &ra, no_io ? 0 : &rio);
        ran[key] += sf == EXEC_OK;
        if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK)
            why = "readable refused, faithful ran";
        else if (sf != EXEC_OK && sr == EXEC_OK)
            why = "readable ran, faithful refused";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
            why = "different io record or channel 12";
        if (why && ++failures <= 3u) {
            unsigned w;
            printf("FAIL case %u key %u: %s (status %d, faithful %d)\n", n, key, why, (int)sr,
                   (int)sf);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
            if (fio.hw.channel12 != rio.hw.channel12)
                printf("  channel 12: %05o, faithful %05o\n", rio.hw.channel12, fio.hw.channel12);
        }
    }
    for (n = 0; n < 4u; ++n)
        if (ran[n] < RUNS / 8u) {
            printf("FAIL: key %u: the faithful routine ran in %u runs\n", n, ran[n]);
            ++failures;
        }
    printf("trim-gimbal-cases: %u runs, faithful runs %u %u %u %u, %u failures\n", RUNS, ran[0],
           ran[1], ran[2], ran[3], failures);
    return failures != 0u;
}
