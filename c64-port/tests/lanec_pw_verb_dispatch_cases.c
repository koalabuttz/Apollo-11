/* SPDX-License-Identifier: MIT */
/* pw-verb-dispatch: VERBFAN to VBFANDIR/VERBTAB/BANKJUMP for verbs 0-39, or
 * to RELDSP for verbs 40-99 (src/dsky_verb_dispatch.c). Generator numbers
 * 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw.h"
#include "dsky_verb_dispatch.h"
#include "dsky_verb_dispatch_cases.h"

static enum lanec_status verb_dispatch_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    enum dsky_verb_route route;
    int ok;
    if (mission == LANEC_LM) {
        p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
        ok = dsky_verb_dispatch(s, &p, &route) == P66_BIAS_OK;
        if (ok) { io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; }
    } else {
        cm_native_io c = {0}; c.superbank = io->superbank; c.interrupt_enabled = io->interrupt_enabled;
        ok = cm_verb_dispatch(s, &c, &route) == EXEC_OK;
        if (ok) { io->superbank = c.superbank; io->interrupt_enabled = c.interrupt_enabled; }
    }
    return ok ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_verb_dispatch_input(s, &p, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, verb_dispatch_run);
}
const lanec_suite lanec_suite_pw_verb_dispatch = { "pw-verb-dispatch", CASES, make, verb_dispatch_run };
