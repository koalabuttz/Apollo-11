/* SPDX-License-Identifier: MIT */
/* pw-nv-busy: ENDIDLE/NVSUBUSY/PRENVBSY/NVSBWAIT/NVSUBSY1 through JOBSLEEP,
 * POODOO or NVSBCOM (src/dsky_nv_busy.c). The generator chooses mission,
 * entry and channel 11 (io->channel[011]); each entry is accepted only at
 * its own Z, so run() takes the first entry the module accepts. */
#include "lanec_pw.h"
#include "dsky_nv_busy.h"
#include "dsky_nv_busy_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    agc_word channel11;
    enum dsky_nv_mission mission;
    enum dsky_nv_busy_entry entry;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_NV_BUSY_CASES);
    lanec_fill(s, io, n);
    dsky_nv_busy_input(s, &p, &channel11, k, &mission, &entry);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_nv_busy_run);
}
const lanec_suite lanec_suite_pw_nv_busy = { "pw-nv-busy", CASES, make, lanec_pw_nv_busy_run };
