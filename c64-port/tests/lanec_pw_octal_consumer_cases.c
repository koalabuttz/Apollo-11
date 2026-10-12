/* SPDX-License-Identifier: MIT */
/* pw-octal-consumer: the DSPA/B/C/AB/ABC row loop (src/dsky_octal_consumer.c)
 * from the generator's verb entry, chaining BEGIN, CHANNEL, FORMAT and
 * RETURN stages (each accepted only at its own Z) until no stage accepts
 * or a stage leaves the loop (ENTRET, alarm or recovery). The CHANNEL stage
 * reads the channel NOUNCADR selects from io->channel[]. The generator
 * chooses the mission (numbers 0-2047 LM, 2048-4095 CM). */
#include "lanec_pw.h"
#include "dsky_octal_consumer.h"
#include "dsky_octal_consumer_cases.h"

static enum lanec_status octal_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p;
    enum dsky_octal_consumer_route route = DSKY_OCTAL_NEXT;
    unsigned step, e, ok = 0;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    for (step = 0; step < 64u && route == DSKY_OCTAL_NEXT; ++step) {
        uint16_t ch = s->words[01017] & 0777u;
        agc_word value = ch < LANEC_CHANNELS ? (agc_word)(io->channel[ch] & AGC_MASK) : 0u;
        for (e = DSKY_OCTAL_BEGIN; e <= DSKY_OCTAL_RETURN; ++e)
            if (dsky_octal_consumer(s, &p, (enum dsky_nv_mission)mission, (enum dsky_octal_consumer_stage)e, value, &route) == P66_BIAS_OK)
                break;
        if (e > DSKY_OCTAL_RETURN) break;
        ok = 1;
    }
    if (!ok) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_OCTAL_CONSUMER_CASES);
    lanec_fill(s, io, n);
    dsky_octal_consumer_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, octal_consumer_run);
}
const lanec_suite lanec_suite_pw_octal_consumer = { "pw-octal-consumer", CASES, make, octal_consumer_run };
