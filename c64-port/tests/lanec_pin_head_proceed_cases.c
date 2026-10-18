/* SPDX-License-Identifier: MIT */
/* pin-head-proceed: VBPROC, VBTERM, PROCKEY, VBRESEQ and RECALTST
 * (src/lanec_pin_head_proceed.c), both missions, from the states
 * pw-load-control's accepted cases reach there (lanec_pin_head_reach), else
 * a filled state with CADRSTOR of each sign. The oracle runs the original to
 * where the head stops. */
#include <string.h>
#include "lanec_pin_head_cases.h"
#include "lanec_pw.h"
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"

static const uint16_t lm_points[] = { LANEC_LM_VBPROC, LANEC_LM_VBTERM, LANEC_LM_PROCKEY, LANEC_LM_VBRESEQ, LANEC_LM_RECALTST };
static const uint16_t cm_points[] = { LANEC_CM_VBPROC, LANEC_CM_VBTERM, LANEC_CM_PROCKEY, LANEC_CM_VBRESEQ, LANEC_CM_RECALTST };
static const uint16_t cadr[] = { 0, 077777u, 3u, 077774u, 01234u, 060000u };
#define POINTS 5u
#define PER 96u
#define CASES (POINTS * 2u * PER)

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t point;
    m->mission = n / POINTS & 1u ? LANEC_CM : LANEC_LM;
    point = (m->mission == LANEC_LM ? lm_points : cm_points)[n % POINTS];
    if (n / (2u * POINTS) % 3u == 2u || !lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_load_control, 040u, point, m->mission, s, io)) {
        lanec_fill(s, io, n);
        lanec_pw_settle(s);
        lanec_enter(s, io, 040u, point);
        s->words[LANEC_LM_CADRSTOR] = cadr[n / (2u * POINTS) % 6u];
    }
    memset(m->range, 0, sizeof m->range);
    m->ranges = 0;
    lanec_pw_exit_stop(m, s, io, lanec_pin_head_proceed_run);
}
const lanec_suite lanec_suite_pin_head_proceed = { "pin-head-proceed", CASES, make, lanec_pin_head_proceed_run };
