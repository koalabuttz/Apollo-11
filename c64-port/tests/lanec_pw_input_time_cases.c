/* SPDX-License-Identifier: MIT */
/* pw-input-time: HMSIN with RND/TST, MPACTST and SIZETST
 * (src/dsky_input_time.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_input_time.h"
#include "dsky_input_time_cases.h"

LANEC_PW_LAMP_RUN(input_time_run, dsky_input_time, cm_input_time, enum dsky_input_time_route, DSKY_INPUT_TIME_LOADED)
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_input_time_input, lanec_pw_lamp_out, input_time_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_input_time = { "pw-input-time", CASES, make, input_time_run };
