/* SPDX-License-Identifier: MIT */
/* pw-operator-request: the operator request entries
 * (src/dsky_operator_request.c) through RELDSP and the request control
 * routes. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_e.h"
#include "dsky_operator_request.h"
#include "dsky_operator_request_cases.h"

#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_operator_request_input, lanec_pw_operator_request_run, CASES)
const lanec_suite lanec_suite_pw_operator_request = { "pw-operator-request", CASES, make, lanec_pw_operator_request_run };
