/* SPDX-License-Identifier: MIT */
/* pw-address-request: the address request entries through their six routes
 * (src/dsky_address_request.c), channel 11 through
 * io->channel[011]. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_a.h"
#include "dsky_address_request.h"
#include "dsky_address_request_cases.h"
static enum dsky_request_route route;
LANEC_PW_CHARIN_RUN(run, dsky_address_request(s, &c, &route), cm_address_request(s, &x, &w, &route))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_address_request_input, 8192u, CASES, run)
const lanec_suite lanec_suite_pw_address_request = { "pw-address-request", CASES, make, run };
