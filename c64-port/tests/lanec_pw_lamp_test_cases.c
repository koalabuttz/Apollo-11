/* SPDX-License-Identifier: MIT */
/* pw-lamp-test: VBTSTLTS, TSTLTS1/2/3 and the post-DSPMM controls, CM
 * CHKPOOH (src/dsky_lamp_test.c). Generator numbers 0-4095 are LM, 4096-8191
 * CM. */
#include "lanec_pw_c.h"
#include "dsky_lamp_test.h"
#include "dsky_lamp_test_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_lamp_test_input, lanec_pw_lamp_out, lanec_pw_lamp_test_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_lamp_test = { "pw-lamp-test", CASES, make, lanec_pw_lamp_test_run };
