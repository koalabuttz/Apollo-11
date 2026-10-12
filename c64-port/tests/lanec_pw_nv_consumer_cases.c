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

static enum lanec_status nv_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p;
    agc_word channel11 = io->channel[011];
    enum dsky_nv_mission nm = (enum dsky_nv_mission)mission;
    unsigned phase;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_nv_consumer_step(s, &p, &channel11, nm, DSKY_NV_CONSUMER_CALL) != P66_BIAS_OK &&
        dsky_nv_consumer_step(s, &p, &channel11, nm, DSKY_NV_CONSUMER_MONITOR) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    for (phase = 1; phase < 5u; ++phase)
        if (dsky_nv_consumer_step(s, &p, &channel11, nm,
                                  (phase & 1u) ? DSKY_NV_CONSUMER_SERVICE : DSKY_NV_CONSUMER_CONTINUE) != P66_BIAS_OK)
            break;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
    return LANEC_OK;
}
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
    lanec_pw_exit_stop(m, s, io, nv_consumer_run);
}
const lanec_suite lanec_suite_pw_nv_consumer = { "pw-nv-consumer", CASES, make, nv_consumer_run };
