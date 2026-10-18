/* SPDX-License-Identifier: MIT */
/* pin-head-nvsub: NVSUB and NVMONOPT (src/lanec_pin_head_nvsub.c), both
 * missions, from the states pw-nv-consumer's accepted cases reach there
 * (lanec_pin_head_reach), a third with DSPLOCK and MONSAVE1 varied, else
 * a filled state that is out of scope. The oracle runs the original to where the head stops. */
#include <string.h>
#include "lanec_pin_head_cases.h"
#include "lanec_pw.h"
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"

#define PER 192u
#define CASES (4u * PER)
static const uint16_t lock[] = { 0, 1u, 077777u, 077776u, 037777u, 040000u };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t point;
    m->mission = n & 2u ? LANEC_CM : LANEC_LM;
    point = (uint16_t)((m->mission == LANEC_LM ? LANEC_LM_NVSUB : LANEC_CM_NVSUB) + (n & 1u));
    if (lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_nv_consumer, 0u, point, m->mission, s, io)) {
        if (n / 4u % 3u == 2u) {   /* the busy and negative paths: DSPLOCK and MONSAVE1 varied */
            s->words[LANEC_LM_DSPLOCK] = lock[n / 12u % 6u];
            if (n / 72u & 1u) s->words[LANEC_LM_MONSAVE1] ^= 020000u;
        }
    } else {   /* Q 0: out of scope (TC Q would leave for no caller's return) */
        lanec_fill(s, io, n);
        lanec_pw_settle(s);
        lanec_enter(s, io, 041u, point);
        s->words[LANEC_Q] = 0;
    }
    memset(m->range, 0, sizeof m->range);
    m->ranges = 0;
    lanec_pw_exit_stop(m, s, io, lanec_pin_head_nvsub_run);
}
const lanec_suite lanec_suite_pin_head_nvsub = { "pin-head-nvsub", CASES, make, lanec_pin_head_nvsub_run };
