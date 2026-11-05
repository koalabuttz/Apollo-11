/* SPDX-License-Identifier: MIT */
/* The port's frozen THROTTLE_CONTROL_ROUTINES cases on the readable
 * routines (make throttle-cases ROPE_LM ROPE_CM).
 *
 * c64-port/tests/test_lm_throttle.c checks the faithful routines against
 * digests of the original program (tests/lm_throttle_vectors.h: 16384
 * cases, each entering THROTTLE, THROTTLE +3, FLATOUT or WHATOUT and run
 * through the trampoline to the caller's return, the 2048 erasable words
 * and channel 14 compared) and checks its own refusals; it also counts the
 * faithful file's paths, which the readable one has not. This runs the
 * same cases and refusals on the readable routines
 * (src/binding/throttle_routines.c), with the ropes loaded that the
 * readable code reads. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_throttle_cases.h"
#include "lm_throttle_vectors.h"

static unsigned checks, failures;
static void check(int ok, const char *what, unsigned n)
{
    ++checks;
    if (!ok && ++failures <= 20u) printf("FAIL %s %u\n", what, n);
}

static agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};

/* The routine at Z, resolved under bank 31 and E-bank 7, must refuse the
 * image as it is, leaving it and channel 14 unchanged. */
static void refused(apollo_overlay_state *s, lm_flight_io *io, const char *what)
{
    apollo_overlay_state before = *s, at = *s;
    agc_word channel14 = io->hw.channel14;
    agc_routine call;

    at.words[4] = LM_THR_FB;
    at.words[3] = 03400u;
    call = agc_transfer_resolve(&program, &at, 0);
    program.hardware = &io->hw;
    check(call && call(&program, s, io) == EXEC_OUT_OF_SCOPE &&
              !memcmp(s, &before, sizeof before) && io->hw.channel14 == channel14,
          what, 0);
}

int main(int argc, char **argv)
{
    apollo_overlay_state s;
    lm_flight_io io;
    lm_thr_case c;
    unsigned n, returned = 0;
    uint16_t steps;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    program.basic = &lm_throttle_routines;
    program.basic_count = 1;
    for (n = 0; n < LM_THR_CASES; ++n) {
        check(lm_thr_digest(&s, (uint16_t)n) == lm_thr_expected[n], "digest", n);
        if (!lm_thr_run(&s, &io, (uint16_t)n, &steps)) ++returned;
    }
    check(returned == LM_THR_CASES, "all cases reach the caller's return", returned);
    lm_thr_input(&s, &io, &c, 0);
    s.words[3] = 01400u;
    s.words[6] = (agc_word)(LM_THR_FB | 1u);
    refused(&s, &io, "THROTTLE under EBANK1");
    lm_thr_input(&s, &io, &c, 2);
    s.words[2] = 01000u;
    refused(&s, &io, "FLATOUT return into erasable");
    lm_thr_input(&s, &io, &c, 3);
    s.words[2] = 050000u;
    refused(&s, &io, "WHATOUT return not a TC");
    lm_thr_input(&s, &io, &c, 0);
    s.words[5] = LM_THR_MASSMULT;
    s.words[2] = 0177u;
    refused(&s, &io, "MASSMULT return into erasable");
    lm_thr_input(&s, &io, &c, 0);
    s.words[5] = LM_THR_FASTCHNG;
    s.words[2] = 0100000u | 02303u;
    refused(&s, &io, "FASTCHNG Q with bit 16");
    lm_thr_input(&s, &io, &c, 0);
    s.words[5] = 02303u;
    s.words[01607u - 01400u + 03400u] = 0;
    refused(&s, &io, "RTNHOLD zero");
    printf("throttle-oracle: %u cases returned; %u checks, %u failures\n", returned, checks,
           failures);
    return failures != 0u;
}
