/* SPDX-License-Identifier: MIT */
/* pw-scale: Luminary DP1OUTSF (41,2710), DP2OUTSF (41,2715) and DP3OUTSF (41,2717)
 * through READLO, TPAGREE and DMP to DSPDCEND (src/dsky_scale.c); the
 * entry is read back from Z. */
#include "lanec_pw_g.h"
#include "dsky_scale.h"
#include "dsky_scale_cases.h"

#define CASES 1024u
/* past the spread: the module's own states with the noun in low erasable
 * (R22DISP at 0314 for noun 49, the CDU counters) */
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, DSKY_SCALE_CASE_COUNT)
                           : (uint16_t)((n - CASES) * 127u % DSKY_SCALE_CASE_COUNT);
    lanec_fill(s, io, n);
    dsky_scale_prepare(s, k);
    if (n >= CASES) lanec_pw_low_noun(s, n - CASES);
    io->superbank = 0100u; io->interrupt_enabled = (uint16_t)(k % 2u);
    lanec_pw_settle(s);
    m->mission = LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_scale_run);
}
const lanec_suite lanec_suite_pw_scale = { "pw-scale", CASES + LANEC_PW_LOW_NOUN, make, lanec_pw_scale_run };
