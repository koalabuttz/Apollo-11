/* SPDX-License-Identifier: MIT */
/* pw-noun-check: COMPTEST/COMPTST1/DCOMPTST/DECTEST/DCTSTCYC/NOUNTEST/GETCOMP
 * (bank 41) through the caller, alarm, recycle or CCSHOLE entry
 * (src/dsky_noun_check.c). Generator numbers 0-4095 are LM, 4096-8191 CM.
 * The routines touch no channel; the restart light is held off. */
#include "lanec_pw.h"
#include "dsky_noun_check.h"
#include "dsky_noun_check_cases.h"

static enum lanec_status noun_check_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    enum dsky_noun_check_route route;
    int ok;
    if (mission == LANEC_LM) {
        dsky_lamp_io p = {{0, 0}, 0, 0, 0, 0, 0};
        p.control.superbank = io->superbank; p.control.interrupt_enabled = io->interrupt_enabled;
        p.channel11 = io->channel[011]; p.channel12 = io->channel[012]; p.channel13 = io->channel[013];
        ok = dsky_noun_check(s, &p, &route) == P66_BIAS_OK;
        if (ok) { io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled; }
    } else {
        cm_native_io c = {0};
        uint16_t wrote = 0, restart = 0;
        c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        ok = cm_noun_check(s, &c, &wrote, &restart, &route) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_lamp_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_noun_check_input(s, &p, k);
    io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, noun_check_run);
}
const lanec_suite lanec_suite_pw_noun_check = { "pw-noun-check", CASES, make, noun_check_run };
