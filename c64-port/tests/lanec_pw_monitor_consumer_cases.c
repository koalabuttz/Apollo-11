/* SPDX-License-Identifier: MIT */
/* pw-monitor-consumer: the compiled MONDO/output/paste stages
 * (src/dsky_monitor_consumer.c) chained from the generator's MONDO display
 * entry: each step runs the stage dsky_monitor_consumer_entry() names for
 * the current Z, until the frontier or a stage the module refuses. The
 * octal CHANNEL stage reads the channel NOUNCADR selects and the decimal
 * stages read channel 33, both from io->channel[]; channel 11 passes
 * through io->channel[011]. Numbers 0-127 LM, 128-255 CM. */
#include "lanec_pw.h"
#include "dsky_monitor_consumer.h"
#include "dsky_monitor_consumer_cases.h"

static enum lanec_status monitor_consumer_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    dsky_charin_io c;
    enum dsky_nv_mission nm = (enum dsky_nv_mission)mission;
    enum dsky_monitor_consumer_stage stage, next;
    unsigned step, ok = 0;
    c.control.superbank = io->superbank; c.control.interrupt_enabled = io->interrupt_enabled;
    c.channel11 = io->channel[011]; c.wrote_channel11 = 0;
    for (step = 0; step < 64u; ++step) {
        agc_word value;
        stage = dsky_monitor_consumer_entry(s, nm);
        if (stage == DSKY_MONC_FRONTIER) break;
        if (stage >= DSKY_MONC_OCTAL_BEGIN && stage <= DSKY_MONC_OCTAL_RETURN) {
            uint16_t ch = s->words[01017] & 0777u;
            value = ch < LANEC_CHANNELS ? (agc_word)(io->channel[ch] & AGC_MASK) : 0u;
        } else value = (agc_word)(io->channel[033] & AGC_MASK);
        if (dsky_monitor_consumer(s, &c, nm, stage, value, &next) != P66_BIAS_OK) break;
        ok = 1;
    }
    if (!ok) return LANEC_OUT_OF_SCOPE;
    io->superbank = c.control.superbank; io->interrupt_enabled = c.control.interrupt_enabled;
    io->channel[011] = c.channel11;
    return LANEC_OK;
}
#define CASES 256u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_charin_io c;
    agc_word channel;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_MONITOR_CONSUMER_CASES);
    lanec_fill(s, io, n);
    dsky_monitor_consumer_input(s, &c, k, &mission, &channel);
    io->superbank = c.control.superbank; io->interrupt_enabled = c.control.interrupt_enabled;
    io->channel[011] = c.channel11;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, monitor_consumer_run);
}
const lanec_suite lanec_suite_pw_monitor_consumer = { "pw-monitor-consumer", CASES, make, monitor_consumer_run };
