/* SPDX-License-Identifier: MIT */
/* pw-mixed-noun: MIXNOUN through VERBFAN or GODSPALM
 * (src/dsky_mixed_noun.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_mixed_noun.h"
#include "dsky_mixed_noun_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, p66_bias_io, dsky_mixed_noun_input, lanec_pw_copy_p66, lanec_pw_mixed_noun_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_mixed_noun = { "pw-mixed-noun", CASES, make, lanec_pw_mixed_noun_run };
