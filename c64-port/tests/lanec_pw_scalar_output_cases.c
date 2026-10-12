/* SPDX-License-Identifier: MIT */
/* pw-scalar-output: the scalar scale bodies through DSPDCEND
 * (src/dsky_scalar_output.c): degrees, arithmetic, arithmetic14, mission
 * angle, LM landing position and CM fraction, chosen by Z as the generator
 * does. Generator numbers 0-10239 are LM, 10240-20479 CM. The generator's
 * channel 33 word goes in channel 033 of the lanec_io. */
#include "lanec_pw.h"
#include "dsky_scalar_output.h"
#include "dsky_scalar_output_cases.h"

static enum lanec_status scalar_output_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static const uint16_t pc[2][6] = {{02615, 02677, 02704, 02623, 02635, 0}, {02516, 02603, 02610, 02524, 0, 02653}};
    unsigned e, mi = mission == LANEC_CM;
    p66_bias_io p;
    for (e = 0; e < 6u && pc[mi][e] != s->words[LANEC_Z]; ++e) {}
    if (e == 6u) return LANEC_OUT_OF_SCOPE;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_scalar_output(s, &p, mi ? DSKY_NV_CM : DSKY_NV_LM, (enum dsky_scalar_output_entry)e, io->channel[033]) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    enum dsky_scalar_output_entry entry;
    agc_word channel;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_SCALAR_OUTPUT_CASES);
    lanec_fill(s, io, n);
    dsky_scalar_output_input(s, &p, k, &mission, &entry, &channel);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    io->channel[033] = channel;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, scalar_output_run);
}
const lanec_suite lanec_suite_pw_scalar_output = { "pw-scalar-output", CASES, make, scalar_output_run };
