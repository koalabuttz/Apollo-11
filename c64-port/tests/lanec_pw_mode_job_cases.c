/* SPDX-License-Identifier: MIT */
/* pw-mode-job: DSPMMJB (LM 40,3534, CM 40,3441) through ENDOFJOB or the
 * CCSHOLE entry (src/dsky_mode_job.c). Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw.h"
#include "dsky_mode_job.h"
#include "dsky_mode_job_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_mode_job_input(s, &p, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_mode_job_run);
}
const lanec_suite lanec_suite_pw_mode_job = { "pw-mode-job", CASES, make, lanec_pw_mode_job_run };
