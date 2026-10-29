/* SPDX-License-Identifier: MIT */
/* pw-enter-zero: ENTPAS0 noun lookup and normal/mixed admission (src/dsky_enter_zero.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_b.h"
#include "dsky_enter_zero.h"
#include "dsky_enter_zero_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_enter_zero_input(s, &p, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_enter_zero_run);
}
const lanec_suite lanec_suite_pw_enter_zero = { "pw-enter-zero", CASES, make, lanec_pw_enter_zero_run };
