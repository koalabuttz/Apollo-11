/* SPDX-License-Identifier: MIT */
/* pin-head-release: RELDSP1 (src/lanec_pin_head_release.c), both
 * missions, from the states pw-monitor's accepted cases reach there
 * (lanec_pin_head_reach), else pw-operator-release's (UNSUSPEN), else a
 * filled state entered with a caller's return in Q. DSPLIST takes each
 * sign in turn, so every CCS path runs. The oracle runs the original to
 * where the head stops. */
#include <string.h>
#include "lanec_pin_head_cases.h"
#include "lanec_pw.h"
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"

#define PER 256u
#define CASES (2u * PER)
static const uint16_t list[] = { 0, 1u, 077777u, 060000u, 012345u, 040000u };
static const uint16_t ret[] = { 03262u, 03504u, 02000u, 07777u, 04000u, 0u, 01777u, 010000u };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t point;
    m->mission = n & 1u ? LANEC_CM : LANEC_LM;
    point = m->mission == LANEC_LM ? LANEC_LM_RELDSP1 : LANEC_CM_RELDSP1;
    if (n / 2u % 4u == 3u ||
        (!lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_monitor, 0u, point, m->mission, s, io) &&
         !lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_operator_release, 0u, point, m->mission, s, io))) {
        lanec_fill(s, io, n);
        lanec_pw_settle(s);
        lanec_enter(s, io, 040u, point);
        s->words[LANEC_Q] = ret[n / 8u % 8u];   /* 0, 01777, 010000: out of scope */
    }
    if (n / 2u % 2u) s->words[LANEC_LM_DSPLIST] = list[n / 4u % 6u];
    memset(m->range, 0, sizeof m->range);
    m->ranges = 0;
    lanec_pw_exit_stop(m, s, io, lanec_pin_head_release_run);
}
const lanec_suite lanec_suite_pin_head_release = { "pin-head-release", CASES, make, lanec_pin_head_release_run };
