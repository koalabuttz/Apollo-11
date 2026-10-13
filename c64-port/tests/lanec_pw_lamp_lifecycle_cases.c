/* SPDX-License-Identifier: MIT */
/* pw-lamp-lifecycle: VBTSTLTS/TSTLTS1-3 lamp test lifecycle through
 * WAITLIST, NOVAC, DSPMM and RELDSP (src/dsky_lamp_lifecycle.c). Generator
 * numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_lamp_lifecycle.h"
#include "dsky_lamp_lifecycle_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_lamp_lifecycle_input, lanec_pw_lamp_out, lanec_pw_lamp_lifecycle_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_lamp_lifecycle = { "pw-lamp-lifecycle", CASES, make, lanec_pw_lamp_lifecycle_run };
