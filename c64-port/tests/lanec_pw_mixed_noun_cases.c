/* SPDX-License-Identifier: MIT */
/* pw-mixed-noun: MIXNOUN through VERBFAN or GODSPALM
 * (src/dsky_mixed_noun.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_mixed_noun.h"
#include "dsky_mixed_noun_cases.h"

LANEC_PW_ROUTE_RUN(mixed_noun_run, dsky_mixed_noun, cm_mixed_noun, enum dsky_mixed_route, DSKY_MIXED_VERB)
#define CASES 1024u
LANEC_PW_MAKE(make, p66_bias_io, dsky_mixed_noun_input, lanec_pw_copy_p66, mixed_noun_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_mixed_noun = { "pw-mixed-noun", CASES, make, mixed_noun_run };
