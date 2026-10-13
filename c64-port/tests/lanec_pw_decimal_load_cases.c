/* SPDX-License-Identifier: MIT */
/* pw-decimal-load: DECDSP's load of the noun component (src/dsky_decimal_load.c).
 * The generator's lower half is LM, the upper half CM. */
#include "lanec_pw_a.h"
#include "dsky_decimal_load.h"
#include "dsky_decimal_load_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission;
    lanec_fill(s, io, n);
    dsky_decimal_load_input(s, &p, (uint16_t)lanec_pw_spread(n, CASES, DSKY_DECIMAL_LOAD_CASES), &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_decimal_load_run);
}
const lanec_suite lanec_suite_pw_decimal_load = { "pw-decimal-load", CASES, make, lanec_pw_decimal_load_run };
