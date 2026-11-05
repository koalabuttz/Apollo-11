/* SPDX-License-Identifier: MIT */
/* The readable LANDING_ANALOG_DISPLAYS against the faithful port's, case
 * by case (make landing-displays-cases ROPE_LM ROPE_CM).
 *
 * Links the readable table (src/binding/landing_displays_routines.c) and
 * the faithful port's under the same name with faithful_ in front (the
 * Makefile renames it). First the keys, which must be the faithful
 * table's. Then the runs: each case is one of the port's test states
 * (c64-port/tests/lm_landing_displays_cases.c), two in three of them
 * entered at any of the four keys, half of them stirred further: LADQSAVE
 * any word or one of DISINDAT's returns, FB, EBANK, overflow in A or L,
 * channels 12, 14 and 30, ALTBITS, ALTSAVE, the velocity monitor's flags,
 * meters and velocities (at, past and inside the ends of the scale), and
 * ITEMP5. Each case runs one call on each side. Where the faithful routine
 * runs, the readable one must too, with the same memory and io record,
 * word for word; where the faithful one refuses, so must the readable one,
 * leaving everything as it found it; and each key's faithful routine must
 * run in at least 1 in 16 of the states entered at it (key_floor.h). */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_landing_displays_cases.h"
#include "key_floor.h"

extern const agc_routine_table faithful_lm_landing_displays_routines;
extern const agc_site_operand faithful_lm_landing_displays_sites[LM_LAD_SITES];

#define CASES 131072u
#define W     s->words
/* The physical word of an E-bank 7 address. */
#define E7(a) (03400u + ((a) - 01400u))

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned tables_differ(void)
{
    unsigned i, failures = 0;
    const agc_routine_table *r = &lm_landing_displays_routines,
                            *f = &faithful_lm_landing_displays_routines;

    if (r->count != f->count) {
        printf("FAIL: %u keys, faithful %u\n", r->count, f->count);
        return 1;
    }
    for (i = 0; i < r->count; ++i)
        if (r->entries[i].physical != f->entries[i].physical) {
            printf("FAIL: key %u is %u, faithful %u\n", i, r->entries[i].physical,
                   f->entries[i].physical);
            ++failures;
        }
    for (i = 0; i < LM_LAD_SITES; ++i)
        if (lm_landing_displays_sites[i].physical !=
                faithful_lm_landing_displays_sites[i].physical ||
            lm_landing_displays_sites[i].word != faithful_lm_landing_displays_sites[i].word) {
            printf("FAIL: site %u differs\n", i);
            ++failures;
        }
    return failures;
}

/* A word for the velocity monitor: near zero, near the scale's ends
 * (MAXVBITS, 0547) or anything. */
static agc_word monitor_word(void)
{
    agc_word v;

    switch (rnd() % 4u) {
    case 0: v = (agc_word)(rnd() % 4u); break;
    case 1: v = (agc_word)(0547u - 3u + rnd() % 7u); break;
    case 2: v = (agc_word)(rnd() % 02000u); break;
    default: return (agc_word)(rnd() & 077777u);
    }
    return rnd() % 2u ? (agc_word)(~v & 077777u) : v;
}

/* Case n: the port's test state n, at another key for two in three, and
 * for odd n more. */
static void state(apollo_overlay_state *s, lm_flight_io *io, unsigned n)
{
    unsigned k = faithful_lm_landing_displays_routines.count, i;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    lm_lad_input(s, io, (uint16_t)(n % LM_LAD_CASES));
    if (n % 3u != 0u)
        W[5] = (agc_word)(faithful_lm_landing_displays_routines.entries[rnd() % k].physical -
                          021u * 1024u + 02000u);
    if (n % 2u == 0u) return;
    switch (rnd() % 4u) { /* LADQSAVE */
    case 0: W[E7(01714u)] = (agc_word)(rnd() & 077777u); break;
    case 1: W[E7(01714u)] = (agc_word)(LM_LAD_ALTROUT + 1u); break;
    case 2: W[E7(01714u)] = (agc_word)(LM_LAD_ALTOUT + 1u); break;
    default: break;
    }
    if (rnd() % 32u == 0u) W[4] = (agc_word)(rnd() & 076000u); /* FB */
    if (rnd() % 32u == 0u) W[3] = (agc_word)(rnd() & 03400u);  /* EBANK */
    if (rnd() % 8u == 0u)                                      /* A with overflow, either way */
        W[0] = (agc_word)(rnd() % 2u ? 040000u | (rnd() & 037777u) : 0137777u & rnd());
    if (rnd() % 16u == 0u)
        W[1] = (agc_word)(rnd() % 2u ? 040000u | (rnd() & 037777u) : 0137777u & rnd());
    if (rnd() % 4u == 0u) io->hw.channel12 = (agc_word)(rnd() & 077777u);
    if (rnd() % 4u == 0u) io->hw.channel14 = (agc_word)(rnd() & 006037u);
    if (rnd() % 4u == 0u) io->hw.channel30 = (agc_word)(rnd() & 077777u);
    if (rnd() % 4u == 0u) /* ALTBITS: -1 (old data), -0, +0 or a new altitude */
        W[E7(01741u)] = (agc_word)(rnd() % 4u == 0u ? 077776u
                                   : rnd() % 2u     ? 077777u
                                                    : rnd() & 077777u);
    if (rnd() % 4u == 0u) W[E7(01712u)] = (agc_word)(rnd() % 4u); /* ALTSAVE */
    for (i = 0; i < 2u; ++i) {
        if (rnd() % 2u) W[E7(01676u) + i] = monitor_word(); /* LATVMETR, FORVMETR */
        if (rnd() % 2u)
            W[E7(01702u) + i] = (agc_word)(rnd() % 3u == 0u ? 1u : rnd() % 2u ? 077776u : 0u);
    }
    if (rnd() % 8u == 0u) W[065] = (agc_word)(rnd() % 2u); /* ITEMP5 */
    W[6] = (agc_word)((W[4] & 076000u) | (W[3] >> 8));     /* BBANK */
}

static uint16_t resolved_point; /* the last run's entry point */
static agc_routine resolved;

static enum exec_status run(const agc_routine_table *table, const agc_site_operand *sites,
                            apollo_overlay_state *s, lm_flight_io *io)
{
    agc_routine_table t[1];
    agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};
    agc_routine call;

    t[0] = *table;
    program.basic = t;
    program.basic_count = 1;
    program.operands = sites;
    program.operand_count = LM_LAD_SITES;
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

        state(&start, &io_start, n);
        fa = ra = start;
        fio = rio = io_start;
        sf = run(&faithful_lm_landing_displays_routines, faithful_lm_landing_displays_sites, &fa,
                 &fio);
        sr = run(&lm_landing_displays_routines, lm_landing_displays_sites, &ra, &rio);
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
            printf("FAIL case %u, Z %05o: %s\n", n, start.words[5], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
            if (memcmp(&fio, &rio, sizeof fio))
                printf("  channels 12 %05o 14 %05o, faithful %05o %05o\n", rio.hw.channel12,
                       rio.hw.channel14, fio.hw.channel12, fio.hw.channel14);
        }
    }
    failures += keys_under_floor("landing-displays-cases");
    printf("landing-displays-cases: %u keys, %u checks (%u run by the faithful routines), %u "
           "failures\n",
           lm_landing_displays_routines.count, CASES, ran, failures);
    return failures != 0;
}
