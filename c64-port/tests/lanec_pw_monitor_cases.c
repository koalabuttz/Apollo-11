/* SPDX-License-Identifier: MIT */
/* pw-monitor: MONITOR/MONREQ/KILLMON/MONDO/MONBUSY bodies through their
 * release, admission, update, TESTNN, ENTRET or END continuations
 * (src/dsky_monitor.c). The generator chooses mission, stage and channel 11
 * (io->channel[011]); each stage is accepted only at its own Z, so run()
 * takes the first stage the module accepts. Past the spread, MONDO with a
 * 16-bit Q: a job entry's Q is whatever the Executive left (LM P47). */
#include "lanec_pw.h"
#include "dsky_monitor.h"
#include "dsky_monitor_cases.h"

#define CASES 2048u
#define MONDO_Q 32u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    dsky_charin_io c;
    enum dsky_nv_mission mission;
    enum dsky_monitor_stage stage;
    uint16_t k = (uint16_t)(n < CASES ? lanec_pw_spread(n, CASES, DSKY_MONITOR_CASES) : (n - CASES) * 211u % DSKY_MONITOR_CASES);
    lanec_fill(s, io, n);
    dsky_monitor_input(s, &c, k, &mission, &stage);
    while (n >= CASES && stage != DSKY_MONITOR_DISPLAY) {
        k = (uint16_t)((k + 1u) % DSKY_MONITOR_CASES);
        dsky_monitor_input(s, &c, k, &mission, &stage);
    }
    if (n >= CASES) s->words[2] = (uint16_t)(n % 2u ? 0177777u : 0100000u | (n * 1237u & 077777u));
    io->superbank = c.control.superbank; io->interrupt_enabled = c.control.interrupt_enabled;
    io->channel[011] = c.channel11;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_monitor_run);
}
const lanec_suite lanec_suite_pw_monitor = { "pw-monitor", CASES + MONDO_Q, make, lanec_pw_monitor_run };
