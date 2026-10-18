/* SPDX-License-Identifier: MIT */
/* pin-head-release: VBPROC +3..+5 and RELDSP1 (src/lanec_pin_head_release.c),
 * both missions. VBPROC's points from the states pw-load-control's accepted
 * cases reach there; RELDSP1 from pw-monitor's (MONIT2), else
 * pw-operator-release's (UNSUSPEN). Every fourth case, and any point no
 * suite reaches, is a filled state entered there, RELDSP1's with a
 * caller's return in Q (some out of scope). DSPLIST takes each sign in
 * turn, so every CCS path runs. The oracle runs the original to where the
 * head stops. */
#include <string.h>
#include "lanec_pin_head_cases.h"
#include "lanec_pw.h"
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"

#define POINTS 4u
#define PER 128u
#define CASES (2u * POINTS * PER)
static const uint16_t list[] = { 0, 1u, 077777u, 060000u, 012345u, 040000u };
static const uint16_t ret[] = { 03262u, 03504u, 02000u, 07777u, 04000u, 0u, 01777u, 010000u };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint32_t k = n / 2u % POINTS, v = n / (2u * POINTS);
    uint16_t point, bank = k < 3u ? 040u : 0u;
    int got;
    m->mission = n & 1u ? LANEC_CM : LANEC_LM;
    point = k < 3u ? (uint16_t)((m->mission == LANEC_LM ? LANEC_LM_VBPROC : LANEC_CM_VBPROC) + 3u + k)
                   : (m->mission == LANEC_LM ? LANEC_LM_RELDSP1 : LANEC_CM_RELDSP1);
    got = v % 4u != 3u &&
          (k < 3u ? lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_load_control, bank, point, m->mission, s, io)
                  : lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_monitor, 0u, point, m->mission, s, io) ||
                        lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_operator_release, 0u, point, m->mission, s, io));
    if (!got) {
        lanec_fill(s, io, n);
        lanec_pw_settle(s);
        lanec_enter(s, io, 040u, point);
        if (k == 3u) s->words[LANEC_Q] = ret[v / 4u % 8u];   /* 0, 01777, 010000: out of scope */
    }
    if (k == 3u && v % 2u) s->words[LANEC_LM_DSPLIST] = list[v / 2u % 6u];
    memset(m->range, 0, sizeof m->range);
    m->ranges = 0;
    lanec_pw_exit_stop(m, s, io, lanec_pin_head_release_run);
}
const lanec_suite lanec_suite_pin_head_release = { "pin-head-release", CASES, make, lanec_pin_head_release_run };
