/* SPDX-License-Identifier: MIT */
/* pw-read-pair: READLO's fresh double-word read (src/dsky_read_pair.c).
 * Generator numbers 0-8191 are LM, 8192-16383 CM. */
#include "lanec_pw_e.h"
#include "dsky_read_pair.h"
#include "dsky_read_pair_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_READ_PAIR_CASES);
    lanec_fill(s, io, n);
    dsky_read_pair_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    LANEC_PW_E_STOP(m, s, io, lanec_pw_read_pair_run);
}
const lanec_suite lanec_suite_pw_read_pair = { "pw-read-pair", CASES, make, lanec_pw_read_pair_run };
