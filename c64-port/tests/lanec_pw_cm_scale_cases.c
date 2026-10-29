/* SPDX-License-Identifier: MIT */
/* pw-cm-scale: Comanche DP1OUTSF (40,2614), DP2OUTSF (40,2621) and DP3OUTSF
 * (40,2623) through READLO, TPAGREE and DMP to DSPDCEND (src/cm_scale.c),
 * the generator's whole 4096-number domain; the entry is read back from Z.
 * DECDSP3 reaches the first two through SFOUTABR and BANKJUMP from bank 41. */
#include "lanec_pw_f.h"
#include "cm_scale_cases.h"

#define CASES CM_SCALE_CASES
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    /* past the spread: the module's own states with the noun in low erasable */
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, CM_SCALE_CASES) : (uint16_t)((n - CASES) * 127u % CM_SCALE_CASES);
    lanec_fill(s, io, n);
    cm_scale_input(s, &c, k);
    if (n >= CASES) lanec_pw_low_noun(s, n - CASES);
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_cm_scale_run);
}
const lanec_suite lanec_suite_pw_cm_scale = { "pw-cm-scale", CASES + LANEC_PW_LOW_NOUN, make, lanec_pw_cm_scale_run };
