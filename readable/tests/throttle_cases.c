/* SPDX-License-Identifier: MIT */
/* The readable THROTTLE_CONTROL_ROUTINES against the faithful port's, case
 * by case (make throttle-cases ROPE_LM ROPE_CM).
 *
 * Links the readable table (src/binding/throttle_routines.c) and the
 * faithful port's under the same name with faithful_ in front (the
 * Makefile renames it). First the keys, which must be the faithful
 * table's. Then the runs: each case is one of the port's test states
 * (c64-port/tests/lm_throttle_cases.c), two in three of them entered at
 * any of the nine keys, half of them stirred further: Q, RTNHOLD and BUF
 * any word or a return out of range, FB, EBANK, the superbank and
 * interrupt latches, overflow in A or L, channel 14, MPAC, PIF and the
 * thrusts' high words. (An erasable word, RTNHOLD or BUF, holds 15 bits;
 * only Q, a register, takes all 16.) Each case runs one call on each side. Where the
 * faithful routine runs, the readable one must too, with the same memory
 * and io record, word for word; where the faithful one refuses, so must
 * the readable one, leaving everything as it found it; and each key's
 * faithful routine must run in at least 1 in 16 of the states entered at
 * it (key_floor.h). */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_throttle_cases.h"
#include "key_floor.h"

extern const agc_routine_table faithful_lm_throttle_routines;

#define CASES 65536u
#define W     s->words

static uint32_t seed = 314159u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned tables_differ(void)
{
    unsigned i, failures = 0;

    if (lm_throttle_routines.count != faithful_lm_throttle_routines.count) {
        printf("FAIL: %u keys, faithful %u\n", lm_throttle_routines.count,
               faithful_lm_throttle_routines.count);
        return 1;
    }
    for (i = 0; i < lm_throttle_routines.count; ++i)
        if (lm_throttle_routines.entries[i].physical !=
            faithful_lm_throttle_routines.entries[i].physical) {
            printf("FAIL: key %u is %u, faithful %u\n", i, lm_throttle_routines.entries[i].physical,
                   faithful_lm_throttle_routines.entries[i].physical);
            ++failures;
        }
    return failures;
}

/* A return word: mostly a TC in fixed memory, sometimes anything. */
static agc_word any_return(void)
{
    switch (rnd() % 4u) {
    case 0: return (agc_word)rnd();
    case 1: return (agc_word)(rnd() & 077777u);
    default: return (agc_word)(02000u + rnd() % 06000u);
    }
}

/* Case n: the port's test state n, at another key for two in three, and
 * for odd n more. */
static void state(apollo_overlay_state *s, lm_flight_io *io, uint16_t n)
{
    lm_thr_case c;
    unsigned k = faithful_lm_throttle_routines.count;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    lm_thr_input(s, io, &c, n);
    if (n % 3u != 0u)
        W[5] = (agc_word)(faithful_lm_throttle_routines.entries[rnd() % k].physical - 031u * 1024u +
                          02000u);
    if (n % 2u == 0u) return;
    if (rnd() % 4u == 0u) W[2] = any_return();                           /* Q */
    if (rnd() % 4u == 0u) W[03607] = (agc_word)(any_return() & 077777u); /* RTNHOLD */
    if (rnd() % 8u == 0u) W[0130] = (agc_word)(any_return() & 077777u);  /* BUF */
    if (rnd() % 32u == 0u) W[4] = (agc_word)(rnd() & 076000u);           /* FB */
    if (rnd() % 32u == 0u) W[3] = (agc_word)(rnd() & 03400u);            /* EBANK */
    if (rnd() % 16u == 0u) io->bias.superbank = (agc_word)(rnd() & 0160u);
    if (rnd() % 16u == 0u) io->bias.interrupt_enabled = (uint16_t)(rnd() % 2u);
    if (rnd() % 8u == 0u) /* A with overflow, either way */
        W[0] = (agc_word)(rnd() % 2u ? 040000u | (rnd() & 037777u) : 0137777u & rnd());
    if (rnd() % 16u == 0u)
        W[1] = (agc_word)(rnd() % 2u ? 040000u | (rnd() & 037777u) : 0137777u & rnd());
    if (rnd() % 4u == 0u) io->hw.channel14 = (agc_word)(rnd() & 077777u);
    if (rnd() % 8u == 0u) W[0154] = (agc_word)(rnd() % 2u ? 0u : 077777u);  /* MPAC */
    if (rnd() % 8u == 0u) W[03612] = (agc_word)(rnd() % 2u ? 0u : 077777u); /* PIF */
    if (rnd() % 8u == 0u) W[03465] = (agc_word)(rnd() & 077777u);           /* FCODD */
    if (rnd() % 8u == 0u) W[03467] = (agc_word)(rnd() & 077777u);           /* FP */
    if (rnd() % 8u == 0u) W[03620] = (agc_word)(rnd() & 077777u);           /* FCOLD */
    W[6] = (agc_word)((W[4] & 076000u) | (W[3] >> 8));                      /* BBANK */
}

static uint16_t resolved_point; /* the last run's entry point */
static agc_routine resolved;

static enum exec_status run(const agc_routine_table *table, apollo_overlay_state *s,
                            lm_flight_io *io)
{
    agc_routine_table t[1];
    agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};
    agc_routine call;

    t[0] = *table;
    program.basic = t;
    program.basic_count = 1;
    program.hardware = &io->hw;
    resolved_point = agc_transfer_next(&program, s, io->bias.superbank, AGC_TRANSFER_NONE, &call);
    resolved = call;
    return call ? call(&program, s, io) : EXEC_OUT_OF_SCOPE;
}

int main(int argc, char **argv)
{
    unsigned n, failures, ran = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures = tables_differ();
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        lm_flight_io io_start, fio, rio;
        enum exec_status sf, sr;
        const char *why = 0;

        state(&start, &io_start, (uint16_t)n);
        fa = ra = start;
        fio = rio = io_start;
        sf = run(&faithful_lm_throttle_routines, &fa, &fio);
        sr = run(&lm_throttle_routines, &ra, &rio);
        if (sf == EXEC_OK) ++ran;
        key_tally(resolved_point, resolved, sf == EXEC_OK);
        if (sr != EXEC_OK &&
            (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io_start, sizeof rio)))
            why = "readable refusal changed the state";
        else if (sf != sr)
            why =
                sf == EXEC_OK ? "readable refused, faithful ran" : "readable ran, faithful refused";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
            why = "different io";
        if (why && ++failures <= 30) {
            unsigned w;
            printf("FAIL case %u, Z %05o Q %06o: %s\n", n, start.words[5], start.words[2], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    failures += keys_under_floor("throttle-cases");
    printf("throttle-cases: %u keys, %u checks (%u run by the faithful routines), %u failures\n",
           lm_throttle_routines.count, CASES, ran, failures);
    return failures != 0;
}
