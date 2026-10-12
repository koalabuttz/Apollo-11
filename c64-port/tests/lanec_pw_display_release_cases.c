/* SPDX-License-Identifier: MIT */
/* pw-display-release: internal RELDSP (LM 04457, CM 04473) through the
 * DSPLIST wake and the Q caller (src/dsky_display_release.c). Channel 011
 * passes through. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_b.h"
#include "dsky_display_release.h"
#include "dsky_display_release_cases.h"

static enum lanec_status display_release_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    int ok;
    if (mission == LANEC_LM) {
        dsky_charin_io p;
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled;
        p.channel11 = io->channel[011]; p.wrote_channel11 = 0;
        ok = dsky_display_release(s, &p) == P66_BIAS_OK;
        if (ok) { io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; io->channel[011] = p.channel11; }
    } else {
        cm_native_io c = {0}; uint16_t wrote = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled; c.channel11 = io->channel[011];
        ok = cm_display_release(s, &c, &wrote) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; io->channel[011] = c.channel11; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_charin_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_display_release_input(s, &p, k);
    io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;
    io->channel[011] = p.channel11;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, display_release_run);
}
const lanec_suite lanec_suite_pw_display_release = { "pw-display-release", CASES, make, display_release_run };
