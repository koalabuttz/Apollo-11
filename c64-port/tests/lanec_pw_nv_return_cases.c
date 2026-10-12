/* SPDX-License-Identifier: MIT */
/* pw-nv-return: NVSUBEND and SUPDXCHZ through the saved fixed caller
 * (src/dsky_nv_return.c). The generator chooses the mission. */
#include "lanec_pw.h"
#include "dsky_nv_return.h"
#include "dsky_nv_return_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_NV_RETURN_CASES);
    lanec_fill(s, io, n);
    dsky_nv_return_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_nv_return_run);
}
const lanec_suite lanec_suite_pw_nv_return = { "pw-nv-return", CASES, make, lanec_pw_nv_return_run };
