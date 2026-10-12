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

static enum lanec_status minutes_output_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static const uint16_t pcs[2][6] = { { 03303, 03343, 03347, 03351, 03361, 03371 },
                                        { 03216, 03256, 03262, 03264, 03274, 03304 } };
    static apollo_overlay_state saved;
    enum dsky_nv_mission m = mission == LANEC_LM ? DSKY_NV_LM : DSKY_NV_CM;
    p66_bias_io p;
    unsigned k;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (k = 0; k < 6u && s->words[LANEC_Z] != pcs[mission][k]; ++k) {}
    if (k == 6u) return LANEC_OUT_OF_SCOPE;
    saved = *s;
    for (; k < 6u; ++k)
        if (s->words[LANEC_Z] != pcs[mission][k] || dsky_minutes_output(s, &p, m, (enum dsky_minutes_stage)k) != P66_BIAS_OK) {
            *s = saved;
            return LANEC_OUT_OF_SCOPE;
        }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_MINUTES_OUTPUT_CASES);
    lanec_fill(s, io, n);
    dsky_minutes_output_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, minutes_output_run);
}
const lanec_suite lanec_suite_pw_minutes_output = { "pw-minutes-output", CASES, make, minutes_output_run };
