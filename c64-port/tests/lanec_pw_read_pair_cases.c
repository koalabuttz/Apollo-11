/* SPDX-License-Identifier: MIT */
/* pw-read-pair: READLO's fresh double-word read (src/dsky_read_pair.c).
 * Generator numbers 0-16383 are LM, 16384-32767 CM. In each half the first
 * 8192 are the module's own cases; the second 8192 repeat them with the pair
 * in low erasable (the clock counters TIME2/TIME1 of noun 36 and other
 * unswitched words from 010), where the read bound 010 is at its edge. */
#include "lanec_pw_e.h"
#include "dsky_read_pair.h"
#include "dsky_read_pair_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    static const uint16_t low[8] = {024u, 010u, 026u, 030u, 034u, 046u, 060u, 076u};
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, 2u * DSKY_READ_PAIR_CASES), j = (uint16_t)(k % 8192u);
    lanec_fill(s, io, n);
    dsky_read_pair_input(s, &p, (uint16_t)(k / 16384u * 8192u + j), &mission);
    if (k % 16384u >= 8192u) s->words[0145] = s->words[0150u + s->words[0117]] = low[(j / 16u) % 8u];
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    LANEC_PW_E_STOP(m, s, io, lanec_pw_read_pair_run);
}
const lanec_suite lanec_suite_pw_read_pair = { "pw-read-pair", CASES, make, lanec_pw_read_pair_run };
