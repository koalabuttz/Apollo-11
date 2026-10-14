/* SPDX-License-Identifier: MIT */
/* pin-head-minsec: Pinball M/SOUT (bank 42), both missions
 * (src/lanec_pin_head_minsec.c), from the decimal-time and Comanche
 * normal-display consumers' and the minutes-seconds modules' states there. */
#include "lanec_pin_head.h"
#include "lanec_pw_runs.h"

#define CASES 8192u
static const lanec_suite *const sources[] = { &lanec_suite_pw_decimal_time_consumer, &lanec_suite_pw_cm_normal_display,
                                              &lanec_suite_pw_minutes_output, &lanec_suite_pw_cm_minutes_seconds, 0 };
static const lanec_pin_head_point points[] = {
    { LANEC_LM, 042u, 03303u }, { LANEC_CM, 042u, 03216u }, { LANEC_LM, 042u, 03343u }, { LANEC_LM, 042u, 03347u }, { LANEC_LM, 042u, 03351u }, { LANEC_LM, 042u, 03361u },
    { LANEC_LM, 042u, 03371u }, { LANEC_CM, 042u, 03256u }, { LANEC_CM, 042u, 03262u }, { LANEC_CM, 042u, 03264u },
    { LANEC_CM, 042u, 03274u }, { LANEC_CM, 042u, 03304u } };
static const lanec_pin_head_family family = { sources, points, 12u, lanec_pin_head_minsec_run };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint32_t round;
    lanec_pin_head_make(&family, n, s, io, m, &round);
    lanec_pw_exit_stop(m, s, io, family.run);
}
const lanec_suite lanec_suite_pin_head_minsec = { "pin-head-minsec", CASES, make, lanec_pin_head_minsec_run };
