/* SPDX-License-Identifier: MIT */
/* pw-scale: Luminary DP1OUTSF (41,2710), DP2OUTSF (41,2715) and DP3OUTSF (41,2717)
 * through READLO, TPAGREE and DMP to DSPDCEND (src/dsky_scale.c); the
 * entry is read back from Z. */
#include "lanec_pw_g.h"
#include "dsky_scale.h"
#include "dsky_scale_cases.h"

LANEC_PW_G_P66_RUN(scale_run, dsky_scale_output(s, &p, s->words[LANEC_Z] == 02710u ? DSKY_DP1OUT
                   : s->words[LANEC_Z] == 02715u ? DSKY_DP2OUT : DSKY_DP3OUT))
#define CASES 1024u
LANEC_PW_G_MAKE(make, dsky_scale_prepare, DSKY_SCALE_CASE_COUNT, CASES, scale_run, io->channel[011])
const lanec_suite lanec_suite_pw_scale = { "pw-scale", CASES, make, scale_run };
