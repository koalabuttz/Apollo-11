/* SPDX-License-Identifier: MIT */
/* pw-decimal: Luminary DSPDECWD (40,3226, rounded) and DSPDECNR (40,3262) through
 * DSPSIGN, SHORTMP and DSPIN to the caller (src/dsky_decimal.c); the
 * rounding is read back from Z. */
#include "lanec_pw_g.h"
#include "dsky_decimal.h"
#include "dsky_decimal_cases.h"

LANEC_PW_G_P66_RUN(decimal_run, dsky_decimal_display(s, &p, (uint8_t)(s->words[LANEC_Z] == 03226u)))
#define CASES 1024u
LANEC_PW_G_MAKE(make, dsky_decimal_prepare, DSKY_DECIMAL_CASE_COUNT, CASES, decimal_run, io->channel[011])
const lanec_suite lanec_suite_pw_decimal = { "pw-decimal", CASES, make, decimal_run };
