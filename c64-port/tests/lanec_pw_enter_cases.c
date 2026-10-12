/* SPDX-License-Identifier: MIT */
/* pw-enter: CHARIN key 034 and ENTER (bank 41, 2002) through pass
 * selection, REQRET and FLASHOFF to ENTPAS0, GODSPALM or REQRET
 * (src/dsky_enter.c). Channel 011 passes through. Generator numbers
 * 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_b.h"
#include "dsky_enter.h"
#include "dsky_enter_cases.h"

static enum lanec_status enter_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    int ok; enum dsky_enter_route r;
    if (mission == LANEC_LM) {
        dsky_charin_io p;
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled;
        p.channel11 = io->channel[011]; p.wrote_channel11 = 0;
        ok = dsky_enter(s, &p, &r) == P66_BIAS_OK;
        if (ok) { io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; io->channel[011] = p.channel11; }
    } else {
        cm_native_io c = {0}; uint16_t wrote = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled; c.channel11 = io->channel[011];
        ok = cm_enter(s, &c, &wrote, &r) == EXEC_OK;
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
    dsky_enter_input(s, &p, k);
    io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;
    io->channel[011] = p.channel11;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, enter_run);
}
const lanec_suite lanec_suite_pw_enter = { "pw-enter", CASES, make, enter_run };
