/* SPDX-License-Identifier: MIT */
/* pw-hours-output: HMSOUT and its helper arithmetic, eight explicit bank 42
 * stages from the fresh entry to the saved ENTRET (src/dsky_hours_output.c).
 * The stage is the one whose source entry Z holds. Each case starts from a
 * generated fresh state (8192 numbers, the generator returns the mission)
 * moved n%8 stages along with the native module, then runs one stage. */
#include "lanec_pw_b.h"
#include "dsky_hours_output.h"
#include "dsky_hours_output_cases.h"

#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_HOURS_OUTPUT_CASES);
    lanec_fill(s, io, n);
    dsky_hours_output_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_advance(s, io, m->mission, lanec_pw_hours_output_run, n % 8u);
    lanec_pw_exit_stop(m, s, io, lanec_pw_hours_output_run);
}
const lanec_suite lanec_suite_pw_hours_output = { "pw-hours-output", CASES, make, lanec_pw_hours_output_run };
