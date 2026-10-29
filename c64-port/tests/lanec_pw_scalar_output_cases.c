/* SPDX-License-Identifier: MIT */
/* pw-scalar-output: the scalar scale bodies through DSPDCEND
 * (src/dsky_scalar_output.c): degrees, arithmetic, arithmetic14, mission
 * angle, LM landing position and CM fraction, chosen by Z as the generator
 * does. Generator numbers 0-10239 are LM, 10240-20479 CM. The generator's
 * channel 33 word goes in channel 033 of the lanec_io. */
#include "lanec_pw.h"
#include "dsky_scalar_output.h"
#include "dsky_scalar_output_cases.h"

#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    enum dsky_scalar_output_entry entry;
    agc_word channel;
    /* past the spread: generator number 8 * 2048 + 1, Comanche OPDEGOUT
     * (40,2524) with MPAC -0, which subtracts 1 before the bias (AD NEG1) */
    /* then the module's own states with the noun in low erasable */
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, DSKY_SCALAR_OUTPUT_CASES)
                 : n == CASES ? 8u * 2048u + 1u : (uint16_t)((n - CASES - 1u) * 127u % DSKY_SCALAR_OUTPUT_CASES);
    lanec_fill(s, io, n);
    dsky_scalar_output_input(s, &p, k, &mission, &entry, &channel);
    if (n > CASES) lanec_pw_low_noun(s, n - CASES - 1u);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    io->channel[033] = channel;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_scalar_output_run);
}
const lanec_suite lanec_suite_pw_scalar_output = { "pw-scalar-output", CASES + 1u + LANEC_PW_LOW_NOUN, make, lanec_pw_scalar_output_run };
