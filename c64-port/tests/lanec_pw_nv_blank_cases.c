/* SPDX-License-Identifier: MIT */
/* pw-nv-blank: BLANKDSP (41,3516) through the ENTSET-2 entry 3566
 * (src/dsky_nv_blank.c). The generator chooses the mission and channel 11
 * (FLASHOFF), which passes through io->channel[011]. */
#include "lanec_pw.h"
#include "dsky_nv_blank.h"
#include "dsky_nv_blank_cases.h"

static enum lanec_status nv_blank_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    p66_bias_io p;
    agc_word channel11 = io->channel[011];
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_nv_blank(s, &p, &channel11, (enum dsky_nv_mission)mission) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
    return LANEC_OK;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    agc_word channel11;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_NV_BLANK_CASES);
    lanec_fill(s, io, n);
    dsky_nv_blank_input(s, &p, &channel11, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled; io->channel[011] = channel11;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, nv_blank_run);
}
const lanec_suite lanec_suite_pw_nv_blank = { "pw-nv-blank", CASES, make, nv_blank_run };
