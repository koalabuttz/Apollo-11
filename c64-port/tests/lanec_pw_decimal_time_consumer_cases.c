/* SPDX-License-Identifier: MIT */
/* pw-decimal-time-consumer: the DECDSP consumer stages for display routines
 * 8 (HMS), 9 (M/S) and 12 (two integers), from the load through the
 * DSPDCEND+2 row continuation (src/dsky_decimal_time_consumer.c). One call
 * runs the stage dsky_decimal_time_consumer_stage() selects from Z. Each
 * case starts from a generated state (4096 numbers, the generator returns
 * the mission) moved n%64 stages along its path with the native module (at
 * most to its last accepted stage), then runs one stage. */
#include "lanec_pw_b.h"
#include "dsky_decimal_time_consumer.h"
#include "dsky_decimal_time_consumer_cases.h"

#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_DECIMAL_TIME_CONSUMER_CASES);
    lanec_fill(s, io, n);
    dsky_decimal_time_consumer_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_advance(s, io, m->mission, lanec_pw_decimal_time_consumer_run, n % 64u);
    lanec_pw_exit_stop(m, s, io, lanec_pw_decimal_time_consumer_run);
}
const lanec_suite lanec_suite_pw_decimal_time_consumer = { "pw-decimal-time-consumer", CASES, make, lanec_pw_decimal_time_consumer_run };
