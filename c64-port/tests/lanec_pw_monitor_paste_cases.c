/* SPDX-License-Identifier: MIT */
/* pw-monitor-paste: PASTEVB/PASTEOPT control stages through UPDATVB-1,
 * BLANKSUB or ENDOFJOB (src/dsky_monitor_paste.c). The generator chooses
 * mission and stage; each stage is accepted only at its own Z, so run()
 * takes the first stage the module accepts. */
#include "lanec_pw.h"
#include "dsky_monitor_paste.h"
#include "dsky_monitor_paste_cases.h"

static enum lanec_status monitor_paste_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p;
    unsigned e;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (e = DSKY_MONITOR_PASTE_BEGIN; e <= DSKY_MONITOR_PASTE_BLANKED; ++e)
        if (dsky_monitor_paste(s, &p, (enum dsky_nv_mission)mission, (enum dsky_monitor_paste_stage)e) == P66_BIAS_OK) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
    return LANEC_OUT_OF_SCOPE;
}
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
    lanec_pw_exit_stop(m, s, io, monitor_paste_run);
}
const lanec_suite lanec_suite_pw_monitor_paste = { "pw-monitor-paste", CASES, make, monitor_paste_run };
