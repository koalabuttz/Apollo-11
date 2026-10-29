/* SPDX-License-Identifier: MIT */
/* pw-octal-display: DSPOCTWD/DSPOCTWO (41,3363) and DSP2BIT (41,3416)
 * through the Q caller or the CCSHOLE entry (src/dsky_octal_display.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw.h"
#include "dsky_octal_display.h"
#include "dsky_octal_display_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 8192u);
    lanec_fill(s, io, n);
    dsky_octal_display_input(s, &p, k);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_octal_display_run);
}
const lanec_suite lanec_suite_pw_octal_display = { "pw-octal-display", CASES, make, lanec_pw_octal_display_run };
