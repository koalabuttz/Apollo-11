/* SPDX-License-Identifier: MIT */
/* pw-enter: CHARIN key 034 and ENTER (bank 41, 2002) through pass
 * selection, REQRET and FLASHOFF to ENTPAS0, GODSPALM or REQRET
 * (src/dsky_enter.c). Channel 011 passes through. Generator numbers
 * 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_b.h"
#include "dsky_enter.h"
#include "dsky_enter_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_charin_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_enter_input(s, &p, k);
    io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;
    io->channel[011] = p.channel11;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_enter_run);
}
const lanec_suite lanec_suite_pw_enter = { "pw-enter", CASES, make, lanec_pw_enter_run };
