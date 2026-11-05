/* SPDX-License-Identifier: MIT */
/* The port's frozen LANDING_ANALOG_DISPLAYS cases on the readable routines
 * (make landing-displays-cases ROPE_LM ROPE_CM).
 *
 * c64-port/tests/test_lm_landing_displays.c checks the faithful routines
 * against digests of the original program
 * (tests/lm_landing_displays_vectors.h: 16384 cases, each entering LANDISP,
 * INTLZE, INTLZE -1 or VARDELAY's return and run through the trampoline
 * until the original program leaves the file, the outcome, channels 12 and
 * 14 and the 2048 erasable words compared) and checks its own refusals; it
 * also counts the faithful file's paths, which the readable one has not.
 * This runs the same cases and refusals on the readable routines
 * (src/binding/landing_displays_routines.c). */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_landing_displays_cases.h"
#include "lm_landing_displays_vectors.h"

static unsigned checks, failures;
static void check(int ok, const char *what, unsigned n)
{
    ++checks;
    if (!ok && ++failures <= 20u) printf("FAIL %s %u\n", what, n);
}

static agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};

/* The routine at Z, resolved as the image stands, must refuse it, leaving
 * it and the io record unchanged. */
static void refused(apollo_overlay_state *s, lm_flight_io *io, agc_routine call, int with_io,
                    const char *what)
{
    apollo_overlay_state before = *s;
    lm_flight_io io_before = *io;

    program.hardware = &io->hw;
    check(call && call(&program, s, with_io ? io : 0) == EXEC_OUT_OF_SCOPE &&
              !memcmp(s, &before, sizeof before) && !memcmp(io, &io_before, sizeof io_before),
          what, 0);
}

int main(int argc, char **argv)
{
    apollo_overlay_state s;
    lm_flight_io io;
    unsigned n, left = 0, refusals = 0;
    uint16_t steps;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    program.basic = &lm_landing_displays_routines;
    program.basic_count = 1;
    for (n = 0; n < LM_LAD_CASES; ++n) {
        unsigned kind;

        check(lm_lad_digest(&s, (uint16_t)n) == lm_lad_expected[n], "digest", n);
        kind = lm_lad_run(&s, &io, (uint16_t)n, &steps);
        if (kind == LM_LAD_LEFT) ++left;
        if (kind == LM_LAD_REFUSED) ++refusals;
        check(kind != LM_LAD_NONE, "every case leaves the file or is refused", n);
    }
    lm_lad_input(&s, &io, 0);
    s.words[3] = 03000u;
    refused(&s, &io, agc_transfer_resolve(&program, &s, 0), 1, "LANDISP under EBANK6");
    {
        agc_routine call;

        lm_lad_input(&s, &io, 1);
        call = agc_transfer_resolve(&program, &s, 0);
        s.words[4] = s.words[6] = 020000u;
        refused(&s, &io, call, 1, "INTLZE under FB 10");
        lm_lad_input(&s, &io, 3);
        refused(&s, &io, agc_transfer_resolve(&program, &s, 0), 0, "VARDELAY's return without io");
    }
    printf("landing-displays-oracle: %u cases left the file, %u refusals; %u checks, %u failures\n",
           left, refusals, checks, failures);
    return failures != 0u;
}
