/* SPDX-License-Identifier: MIT */
/* pw-load-request: ABCLOAD/ABLOAD/ALOAD/BLOAD/CLOAD and the next-component
 * requests (src/dsky_load_request.c). Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_load_request.h"
#include "dsky_load_request_cases.h"

LANEC_PW_LAMP_RUN(load_request_run, dsky_load_request, cm_load_request, enum dsky_load_request_route, DSKY_LOAD_REQUEST_END)
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_load_request_input, lanec_pw_lamp_out, load_request_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_load_request = { "pw-load-request", CASES, make, load_request_run };
