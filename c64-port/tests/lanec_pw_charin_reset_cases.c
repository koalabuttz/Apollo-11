/* SPDX-License-Identifier: MIT */
/* pw-charin-reset: CHARIN RSET (key 022) through ERROR and ENDOFJOB
 * (src/dsky_charin_reset.c). Generator numbers 0-4095 are LM, 4096-8191 CM.
 * Channels 11 and 13 pass through io->channel[011] and [013]. The RESTART
 * light the module turns off is Virtual AGC's RestartLight flip-flop
 * (cleared by writing channel 11 bit 10), outside the compared state, so
 * run() passes it as off and drops the module's result. */
#include "lanec_pw.h"
#include "dsky_charin_reset.h"
#include "dsky_charin_reset_cases.h"

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    int ok;
    uint16_t c11, c13;
    if (mission == LANEC_LM) {
        dsky_reset_io r;
        r.control.superbank = io->superbank; r.control.interrupt_enabled = io->interrupt_enabled;
        r.channel11 = io->channel[011]; r.channel13 = io->channel[013]; r.wrote_channels = 0;
        r.restart_light = 0;
        ok = dsky_charin_reset(s, &r) == P66_BIAS_OK;
        if (ok) { io->superbank = r.control.superbank; io->interrupt_enabled = r.control.interrupt_enabled; }
        c11 = r.channel11; c13 = r.channel13;
    } else {
        cm_native_io c = {0};
        uint16_t wrote = 0, light = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        c.channel11 = io->channel[011]; c.channel13 = io->channel[013];
        ok = cm_charin_reset(s, &c, &wrote, &light) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
        c11 = c.channel11; c13 = c.channel13;
    }
    if (!ok) return LANEC_OUT_OF_SCOPE;
    io->channel[011] = c11; io->channel[013] = c13;
    return LANEC_OK;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_reset_io r;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_charin_reset_input(s, &r, k);
    io->superbank = r.control.superbank; io->interrupt_enabled = r.control.interrupt_enabled;
    io->channel[011] = r.channel11; io->channel[013] = r.channel13;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, run);
}
const lanec_suite lanec_suite_pw_charin_reset = { "pw-charin-reset", CASES, make, run };
