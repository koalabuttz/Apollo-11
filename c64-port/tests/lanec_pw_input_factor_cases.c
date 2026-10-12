/* SPDX-License-Identifier: MIT */
/* pw-input-factor: PUTDECSF/PUTSFNOR/PUTDCSF2 through GTSFIN (src/dsky_input_factor.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_b.h"
#include "dsky_input_factor.h"
#include "dsky_input_factor_cases.h"

LANEC_PW_LAMP_RUN(input_factor_run, dsky_input_factor, cm_input_factor, enum dsky_input_factor_route)
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_lamp_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_input_factor_input(s, &p, k);
    io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;
    io->channel[011] = p.channel11; io->channel[012] = p.channel12; io->channel[013] = p.channel13;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, input_factor_run);
}
const lanec_suite lanec_suite_pw_input_factor = { "pw-input-factor", CASES, make, input_factor_run };
