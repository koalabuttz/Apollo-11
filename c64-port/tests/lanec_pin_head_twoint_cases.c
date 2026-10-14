/* SPDX-License-Identifier: MIT */
/* pin-head-twoint: Pinball 2INTOUT +1, +2, +4 and +013 (bank 40), both
 * missions (src/lanec_pin_head_twoint.c), from the decimal-time consumer's
 * states there. */
#include "lanec_pin_head.h"
#include "lanec_pw_runs.h"
#include "lanec_symbols.h"

#define CASES 8192u
static const lanec_suite *const sources[] = { &lanec_suite_pw_decimal_time_consumer, &lanec_suite_pw_two_integer, 0 };
static const lanec_pin_head_point points[] = {
    { LANEC_LM, 040u, LANEC_LM_2INTOUT + 1u }, { LANEC_LM, 040u, LANEC_LM_2INTOUT + 2u },
    { LANEC_LM, 040u, LANEC_LM_2INTOUT + 4u }, { LANEC_LM, 040u, LANEC_LM_2INTOUT + 013u },
    { LANEC_CM, 040u, LANEC_CM_2INTOUT + 1u }, { LANEC_CM, 040u, LANEC_CM_2INTOUT + 2u },
    { LANEC_CM, 040u, LANEC_CM_2INTOUT + 4u }, { LANEC_CM, 040u, LANEC_CM_2INTOUT + 013u } };
static const lanec_pin_head_family family = { sources, points, 8u, lanec_pin_head_twoint_run };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint32_t round;
    lanec_pin_head_make(&family, n, s, io, m, &round);
    lanec_pw_exit_stop(m, s, io, family.run);
}
const lanec_suite lanec_suite_pin_head_twoint = { "pin-head-twoint", CASES, make, lanec_pin_head_twoint_run };
