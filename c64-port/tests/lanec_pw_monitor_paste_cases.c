/* SPDX-License-Identifier: MIT */
/* pw-monitor-paste: PASTEVB/PASTEOPT control stages through UPDATVB-1,
 * BLANKSUB or ENDOFJOB (src/dsky_monitor_paste.c). The generator chooses
 * mission and stage; each stage is accepted only at its own Z, so run()
 * takes the first stage the module accepts. */
#include "lanec_pw.h"
#include "dsky_monitor_paste.h"
#include "dsky_monitor_paste_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    enum dsky_monitor_paste_stage stage;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_MONITOR_PASTE_CASES);
    lanec_fill(s, io, n);
    dsky_monitor_paste_input(s, &p, k, &mission, &stage);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_monitor_paste_run);
}
const lanec_suite lanec_suite_pw_monitor_paste = { "pw-monitor-paste", CASES, make, lanec_pw_monitor_paste_run };
