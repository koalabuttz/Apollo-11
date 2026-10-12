/* SPDX-License-Identifier: MIT */
/* pw-mode-admit: DSPMM through NOVAC/RELINT to the caller, or BAILOUT
 * (src/dsky_mode_admit.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_mode_admit.h"
#include "dsky_mode_admit_cases.h"

LANEC_PW_ROUTE_RUN(mode_admit_run, dsky_mode_admit, cm_mode_admit, enum dsky_mode_admit_route, DSKY_MODE_ADMIT_CALLER)
#define CASES 1024u
LANEC_PW_MAKE(make, p66_bias_io, dsky_mode_admit_input, lanec_pw_copy_p66, mode_admit_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_mode_admit = { "pw-mode-admit", CASES, make, mode_admit_run };
