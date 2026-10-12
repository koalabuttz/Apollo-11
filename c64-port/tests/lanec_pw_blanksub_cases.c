/* SPDX-License-Identifier: MIT */
/* pw-blanksub: BLANKSUB (LM 4255, CM 4271) through BLNKSUB1, 5BLANK and the
 * SUPDXCHZ return (src/dsky_blanksub.c). Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw.h"
#include "dsky_blanksub.h"
#include "dsky_blanksub_cases.h"

LANEC_PW_P66_RUN(blanksub_run, dsky_blanksub, cm_blanksub)
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_blanksub_input(s, &p, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, blanksub_run);
}
const lanec_suite lanec_suite_pw_blanksub = { "pw-blanksub", CASES, make, blanksub_run };
