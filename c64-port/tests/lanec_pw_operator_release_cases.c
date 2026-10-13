/* SPDX-License-Identifier: MIT */
/* pw-operator-release: TSTLTS4 operator release (src/dsky_operator_release.c)
 * through RELDSP/JOBWAKE to ENDOFJOB, PINBRNCH or BLNKSUB1. Generator numbers
 * 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_e.h"
#include "dsky_operator_release.h"
#include "dsky_operator_release_cases.h"

#define CASES 1024u
LANEC_PW_LAMP_MAKE(make, dsky_operator_release_input, lanec_pw_operator_release_run, CASES)
const lanec_suite lanec_suite_pw_operator_release = { "pw-operator-release", CASES, make, lanec_pw_operator_release_run };
