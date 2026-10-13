/* SPDX-License-Identifier: MIT */
/* pw-nv-consumer: NVSUB/NVMONOPT admission, the selected verb/noun/blank
 * service and its source continuation, chained as the module's own check
 * does (src/dsky_nv_consumer.c): CALL or MONITOR, then SERVICE and
 * CONTINUE alternately while the module accepts the next stage (each stage
 * is accepted only at its own Z). Channel 11 passes through
 * io->channel[011]. The generator chooses the mission. */
#include "lanec_pw.h"
#include "dsky_nv_consumer.h"
#include "dsky_nv_consumer_cases.h"

#define CASES 1792u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    agc_word channel11;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_NV_CONSUMER_CASES);
    lanec_fill(s, io, n);
    dsky_nv_consumer_input(s, &p, &channel11, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_nv_consumer_run);
}
const lanec_suite lanec_suite_pw_nv_consumer = { "pw-nv-consumer", CASES, make, lanec_pw_nv_consumer_run };
