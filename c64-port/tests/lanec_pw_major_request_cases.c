/* SPDX-License-Identifier: MIT */
/* pw-major-request: MMCHANG, its operator return or REQMM through the
 * ENDINST/NVSBENDL/MONBACK, V37 and ALMCYCLE handoffs
 * (src/dsky_major_request.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_major_request.h"
#include "dsky_major_request_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_charin_io, dsky_major_request_input, lanec_pw_charin_out, lanec_pw_major_request_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_major_request = { "pw-major-request", CASES, make, lanec_pw_major_request_run };
