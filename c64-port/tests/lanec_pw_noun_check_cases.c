/* SPDX-License-Identifier: MIT */
/* pw-noun-check: COMPTEST/COMPTST1/DCOMPTST/DECTEST/DCTSTCYC/NOUNTEST/GETCOMP
 * (bank 41) through the caller, alarm, recycle or CCSHOLE entry
 * (src/dsky_noun_check.c). Generator numbers 0-4095 are LM, 4096-8191 CM.
 * The routines touch no channel; the restart light is held off. */
#include "lanec_pw.h"
#include "dsky_noun_check.h"
#include "dsky_noun_check_cases.h"

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
    lanec_pw_exit_stop(m, s, io, lanec_pw_noun_check_run);
}
const lanec_suite lanec_suite_pw_noun_check = { "pw-noun-check", CASES, make, lanec_pw_noun_check_run };
