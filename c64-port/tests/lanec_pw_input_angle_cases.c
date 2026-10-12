/* SPDX-License-Identifier: MIT */
/* pw-input-angle: DEGINSF/SIGNFIX/NEG180 angle input scaling (src/dsky_input_angle.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_b.h"
#include "dsky_input_angle.h"
#include "dsky_input_angle_cases.h"

LANEC_PW_LAMP_RUN(input_angle_run, dsky_input_angle, cm_input_angle, enum dsky_input_angle_route)
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_lamp_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_input_angle_input(s, &p, k);
    io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;
    io->channel[011] = p.channel11; io->channel[012] = p.channel12; io->channel[013] = p.channel13;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, input_angle_run);
}
const lanec_suite lanec_suite_pw_input_angle = { "pw-input-angle", CASES, make, input_angle_run };
