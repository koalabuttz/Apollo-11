/* SPDX-License-Identifier: MIT */
/* pw-decimal-consumer: actual DECDSP from its entry through every row to
 * ENTRET or the CCS-hole recovery (src/dsky_decimal_consumer.c), its
 * stages chained as tools/dsky_decimal_consumer_snapshot.c chains them:
 * LOAD, SELECT, SCALAR or DP (by the scale routine), CALL, FORMAT, RETURN,
 * then SELECT again for the next row. Generator numbers 0-2047 are LM,
 * 2048-4095 CM; channel 33 passes through io->channel[033]. */
#include "lanec_pw_a.h"
#include "dsky_decimal_consumer.h"
#include "dsky_decimal_consumer_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission; agc_word channel;
    lanec_fill(s, io, n);
    dsky_decimal_consumer_input(s, &p, (uint16_t)lanec_pw_spread(n, CASES, DSKY_DECIMAL_CONSUMER_CASES), &mission, &channel);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    io->channel[033] = channel;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_decimal_consumer_run);
}
const lanec_suite lanec_suite_pw_decimal_consumer = { "pw-decimal-consumer", CASES, make, lanec_pw_decimal_consumer_run };
