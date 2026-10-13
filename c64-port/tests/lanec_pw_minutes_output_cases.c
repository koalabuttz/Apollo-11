/* SPDX-License-Identifier: MIT */
/* pw-minutes-output: M/SOUT from its fresh entry through the clamp, the
 * seconds and minutes scaling, both signed two-character displays and the
 * middle blank to the DSPDCEND+2 reentry (src/dsky_minutes_output.c). The
 * native module is entered once per stage, as its own snapshot test does;
 * run() chains the stages from the one Z names through the last. Generator
 * numbers 0-8191, each choosing its own mission. */
#include "lanec_pw.h"
#include "dsky_minutes_output.h"
#include "dsky_minutes_output_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_MINUTES_OUTPUT_CASES);
    lanec_fill(s, io, n);
    if (k < DSKY_MINUTES_OUTPUT_CASES) dsky_minutes_output_input(s, &p, k, &mission);
    else {    /* MPAC = +-025 exactly with MPAC+1 = +-0: the CCS MPAC +1 tie paths to M/SNORM */
        static const uint16_t pair[4][2] = { { 025, 0 }, { 025, 077777 }, { 077752, 077777 }, { 077752, 0 } };
        uint16_t i = (uint16_t)(k - DSKY_MINUTES_OUTPUT_CASES);
        dsky_minutes_output_input(s, &p, (uint16_t)(i < 4u ? 0u : 4096u), &mission);
        s->words[s->words[0145]] = pair[i % 4u][0];
        s->words[s->words[0145] + 1u] = pair[i % 4u][1];
    }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_minutes_output_run);
}
const lanec_suite lanec_suite_pw_minutes_output = { "pw-minutes-output", CASES + 8u, make, lanec_pw_minutes_output_run };
