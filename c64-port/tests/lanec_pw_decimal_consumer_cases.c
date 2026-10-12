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

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p;
    enum dsky_decimal_consumer_route route = DSKY_DECIMAL_NEXT;
    agc_word routine = (agc_word)(s->words[0140] == 1u ? (s->words[0147] >> 5) & 037u : s->words[0153] & 037u);
    unsigned phase = 0, steps;
    static apollo_overlay_state before;
    before = *s;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (steps = 0; steps < 64u; ++steps) {
        enum dsky_decimal_consumer_stage stage =
            phase == 0u ? DSKY_DECIMAL_LOAD : phase == 1u ? DSKY_DECIMAL_SELECT
            : phase == 2u ? (routine == 4u || routine == 5u || routine == 7u || routine == 10u ? DSKY_DECIMAL_DP : DSKY_DECIMAL_SCALAR)
            : phase == 3u ? DSKY_DECIMAL_CALL : phase == 4u ? DSKY_DECIMAL_FORMAT : DSKY_DECIMAL_RETURN;
        /* A stage refused anywhere in the chain refuses the whole case. */
        if (dsky_decimal_consumer(s, &p, lanec_pw_nv(mission), stage, io->channel[033], &route) != P66_BIAS_OK) break;
        if (route != DSKY_DECIMAL_NEXT) {
            io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
            return LANEC_OK;
        }
        phase = phase == 5u ? 1u : phase + 1u;
        if (phase == 2u && routine == 1u) phase = 3u;
    }
    *s = before;
    return LANEC_OUT_OF_SCOPE;
}
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
    lanec_pw_exit_stop(m, s, io, run);
}
const lanec_suite lanec_suite_pw_decimal_consumer = { "pw-decimal-consumer", CASES, make, run };
