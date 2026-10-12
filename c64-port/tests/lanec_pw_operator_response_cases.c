/* SPDX-License-Identifier: MIT */
/* pw-operator-response: the operator response entries
 * (src/dsky_operator_response.c). Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw_e.h"
#include "dsky_operator_response.h"
#include "dsky_operator_response_cases.h"

LANEC_PW_LAMP_RUN(operator_response_run, dsky_operator_response(s, &p),
                  cm_operator_response(s, &c, &wrote, &restart))
#define CASES 1024u
LANEC_PW_LAMP_MAKE(make, dsky_operator_response_input, operator_response_run, CASES)
const lanec_suite lanec_suite_pw_operator_response = { "pw-operator-response", CASES, make, operator_response_run };
