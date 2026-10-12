/* SPDX-License-Identifier: MIT */
/* pw-lm-pinball-busy: Luminary NVSUBUSY (4442) through POSTJUMP/NVSUBSY1 to
 * JOBSLEEP or the inline POODOO alarm 01206 (src/lm_pinball_busy.c); both
 * boundaries are accepted results. */
#include "lanec_pw.h"
#include "lm_busy_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    agc_word channel11;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, LM_BUSY_CASES);
    lanec_fill(s, io, n);
    lm_busy_input(s, &p, &channel11, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    io->channel[011] = channel11;
    lanec_pw_settle(s);
    m->mission = LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_lm_pinball_busy_run);
}
const lanec_suite lanec_suite_pw_lm_pinball_busy = { "pw-lm-pinball-busy", CASES, make, lanec_pw_lm_pinball_busy_run };
