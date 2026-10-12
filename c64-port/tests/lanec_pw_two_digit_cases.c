/* SPDX-License-Identifier: MIT */
/* pw-two-digit: Luminary DSPDECVN (40,3306) through SHORTMP and two DSPIN passes to
 * the caller (src/dsky_two_digit.c). */
#include "lanec_pw_g.h"
#include "dsky_two_digit.h"
#include "dsky_two_digit_cases.h"

LANEC_PW_G_P66_RUN(two_d_run, dsky_two_digit(s, &p))
#define CASES 1024u
LANEC_PW_G_MAKE(make, dsky_two_digit_prepare, DSKY_TWO_DIGIT_CASE_COUNT, CASES, two_d_run, io->channel[011])
const lanec_suite lanec_suite_pw_two_digit = { "pw-two-digit", CASES, make, two_d_run };
