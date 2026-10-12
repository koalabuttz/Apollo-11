/* SPDX-License-Identifier: MIT */
/* pw-charin-reset: CHARIN RSET (key 022) through ERROR and ENDOFJOB
 * (src/dsky_charin_reset.c). Generator numbers 0-4095 are LM, 4096-8191 CM.
 * Channels 11 and 13 pass through io->channel[011] and [013]. The RESTART
 * light the module turns off is Virtual AGC's RestartLight flip-flop
 * (cleared by writing channel 11 bit 10), outside the compared state, so
 * lanec_pw_charin_reset_run() passes it as off and drops the module's result. */
#include "lanec_pw.h"
#include "dsky_charin_reset.h"
#include "dsky_charin_reset_cases.h"

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
    lanec_pw_exit_stop(m, s, io, lanec_pw_charin_reset_run);
}
const lanec_suite lanec_suite_pw_charin_reset = { "pw-charin-reset", CASES, make, lanec_pw_charin_reset_run };
