/* SPDX-License-Identifier: MIT */
/* pw-input-scale: the arithmetic/DP input-scale bodies through PUTCOM2 (src/dsky_input_scale.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_b.h"
#include "dsky_input_scale.h"
#include "dsky_input_scale_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_lamp_io p;
    /* past the spread: the module's own states with the target in low
     * erasable */
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, 8192u) : (uint16_t)((n - CASES) * 127u % 8192u);
    lanec_fill(s, io, n);
    dsky_input_scale_input(s, &p, k);
    if (n >= CASES) lanec_pw_low_noun(s, n - CASES);
    io->superbank = p.control.superbank; io->interrupt_enabled = p.control.interrupt_enabled;
    io->channel[011] = p.channel11; io->channel[012] = p.channel12; io->channel[013] = p.channel13;
    lanec_pw_settle(s);
    m->mission = k >= 4096u ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_input_scale_run);
}
const lanec_suite lanec_suite_pw_input_scale = { "pw-input-scale", CASES + LANEC_PW_LOW_NOUN, make, lanec_pw_input_scale_run };
