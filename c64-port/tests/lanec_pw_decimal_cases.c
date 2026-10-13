/* SPDX-License-Identifier: MIT */
/* pw-decimal: Luminary DSPDECWD (40,3226, rounded) and DSPDECNR (40,3262) through
 * DSPSIGN, SHORTMP and DSPIN to the caller (src/dsky_decimal.c); the
 * rounding is read back from Z. */
#include "lanec_pw_g.h"
#include "dsky_decimal.h"
#include "dsky_decimal_cases.h"

#define CASES 1024u
LANEC_PW_G_MAKE(make, dsky_decimal_prepare, DSKY_DECIMAL_CASE_COUNT, CASES, lanec_pw_decimal_run, io->channel[011])
const lanec_suite lanec_suite_pw_decimal = { "pw-decimal", CASES, make, lanec_pw_decimal_run };
