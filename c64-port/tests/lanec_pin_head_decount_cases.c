/* SPDX-License-Identifier: MIT */
/* pin-head-decount: Pinball DSPDCEND +2 (41,2570), both missions
 * (src/lanec_pin_head_decount.c). The states the decimal-time and Comanche
 * display consumers took there (DECOUNT +0, the last component), each also
 * with DECOUNT 1, 2, -1 and -0, so every CCS outcome runs. */
#include "lanec_pin_head.h"
#include "lanec_pw_runs.h"

#define CASES 8192u
static const lanec_suite *const sources[] = { &lanec_suite_pw_decimal_time_consumer, &lanec_suite_pw_cm_dynamic_display,
                                              &lanec_suite_pw_cm_normal_display, &lanec_suite_pw_cm_nv85, 0 };
static const lanec_pin_head_point points[] = { { LANEC_LM, 041u, 02570u }, { LANEC_CM, 041u, 02570u } };
static const lanec_pin_head_family family = { sources, points, 2u, lanec_pin_head_decount_run };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    static const uint16_t decount[4] = { 1u, 2u, 077776u, 077777u };
    uint32_t round;
    lanec_pin_head_make(&family, n, s, io, m, &round);
    if (round % 5u) s->words[0117] = decount[round % 5u - 1u];
    lanec_pw_exit_stop(m, s, io, family.run);
}
const lanec_suite lanec_suite_pin_head_decount = { "pin-head-decount", CASES, make, lanec_pin_head_decount_run };
