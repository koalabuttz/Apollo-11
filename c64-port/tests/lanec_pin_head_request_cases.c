/* SPDX-License-Identifier: MIT */
/* pin-head-request: VBRQEXEC and VBRQWAIT (src/lanec_pin_head_request.c),
 * both missions, from the states pw-operator-request's accepted cases reach
 * there (lanec_pin_head_reach), else a filled state with DSPTEM1 bit 1 either
 * way. The oracle runs the original to where the head stops. */
#include <string.h>
#include "lanec_pin_head_cases.h"
#include "lanec_pw.h"
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"

#define PER 128u
#define CASES (4u * PER)

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t point = n & 1u ? LANEC_LM_VBRQWAIT : LANEC_LM_VBRQEXEC;
    m->mission = n & 2u ? LANEC_CM : LANEC_LM;
    if (n / 4u % 4u == 3u || !lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_operator_request, 041u, point, m->mission, s, io)) {
        lanec_fill(s, io, n);
        lanec_pw_settle(s);
        lanec_enter(s, io, 041u, point);
        if (n / 16u & 1u) s->words[LANEC_LM_DSPTEM1] ^= 1u;
    }
    memset(m->range, 0, sizeof m->range);
    m->ranges = 0;
    lanec_pw_exit_stop(m, s, io, lanec_pin_head_request_run);
}
const lanec_suite lanec_suite_pin_head_request = { "pin-head-request", CASES, make, lanec_pin_head_request_run };
