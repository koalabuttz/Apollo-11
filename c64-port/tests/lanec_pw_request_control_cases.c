/* SPDX-License-Identifier: MIT */
/* pw-request-control: the request control entries (src/dsky_request_control.c)
 * to RELDSP, NOVAC, FINDVAC or WAITLIST. Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw_e.h"
#include "dsky_request_control.h"
#include "dsky_request_control_cases.h"

static enum dsky_request_control_route route;
LANEC_PW_CHARIN_RUN(request_control_run, dsky_request_control(s, &p, &route),
                    cm_request_control(s, &c, &wrote, &route))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_request_control_input, request_control_run, CASES)
const lanec_suite lanec_suite_pw_request_control = { "pw-request-control", CASES, make, request_control_run };
