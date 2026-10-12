/* SPDX-License-Identifier: MIT */
/* pw-lamp-release: TSTLTS4 through RELDSP/JOBWAKE to ENDOFJOB, PINBRNCH or
 * BLNKSUB1 (src/dsky_lamp_release.c). Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_lamp_release.h"
#include "dsky_lamp_release_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_lamp_release_input, lanec_pw_lamp_out, lanec_pw_lamp_release_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_lamp_release = { "pw-lamp-release", CASES, make, lanec_pw_lamp_release_run };
