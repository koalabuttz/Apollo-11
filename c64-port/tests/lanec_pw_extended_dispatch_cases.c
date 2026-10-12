/* SPDX-License-Identifier: MIT */
/* pw-extended-dispatch: VERBFAN/GOEXTVB extended-verb dispatch to the selected verb body (src/dsky_extended_dispatch.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM.
 * Known: for VERB69 (MPAC 29) the native module stops after one turn of the
 * real "VERB69 TC VERB69" loop (33,2037, Q 2040), as its own reference does;
 * the oracle stops at the first arrival at 33,2037 (Q 2002), so those 17
 * cases differ in Q only. */
#include "lanec_pw.h"
#include "dsky_extended_dispatch.h"
#include "dsky_extended_dispatch_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_extended_dispatch_input(s, &p, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_extended_dispatch_run);
}
const lanec_suite lanec_suite_pw_extended_dispatch = { "pw-extended-dispatch", CASES, make, lanec_pw_extended_dispatch_run };
