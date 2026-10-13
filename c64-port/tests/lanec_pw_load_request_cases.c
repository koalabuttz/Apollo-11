/* SPDX-License-Identifier: MIT */
/* pw-load-request: ABCLOAD/ABLOAD/ALOAD/BLOAD/CLOAD and the next-component
 * requests (src/dsky_load_request.c). Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_load_request.h"
#include "dsky_load_request_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_load_request_input, lanec_pw_lamp_out, lanec_pw_load_request_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_load_request = { "pw-load-request", CASES, make, lanec_pw_load_request_run };
