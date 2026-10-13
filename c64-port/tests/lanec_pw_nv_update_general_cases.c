/* SPDX-License-Identifier: MIT */
/* pw-nv-update-general: the general UPDATVB/UPDATNN fronts through the
 * fixed caller or the CCSHOLE entry (src/dsky_nv_update_general.c). The
 * generator chooses mission and entry; each entry is accepted only at its
 * own Z, so run() takes the first entry the module accepts. */
#include "lanec_pw.h"
#include "dsky_nv_update_general.h"
#include "dsky_nv_update_general_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    enum dsky_nv_update_entry entry;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_NV_UPDATE_GENERAL_CASES);
    lanec_fill(s, io, n);
    dsky_nv_update_general_input(s, &p, k, &mission, &entry);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_nv_update_general_run);
}
const lanec_suite lanec_suite_pw_nv_update_general = { "pw-nv-update-general", CASES, make, lanec_pw_nv_update_general_run };
