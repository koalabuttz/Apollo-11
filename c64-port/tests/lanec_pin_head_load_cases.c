/* SPDX-License-Identifier: MIT */
/* pin-head-load: every point of src/lanec_pin_head_load.c, both missions.
 * An entry's states are the accepted cases of the module the head replaces
 * (pw-load-request, pw-load-complete) at that entry; a return point's are
 * those cases run through the Pinball routines (lanec_pin_chain) to it, so
 * the round trips (pin-transfer) start where the callers' states are. A
 * point no case reaches takes a filled state. The oracle runs the original
 * from the same state to where the head stops (lanec_pw_exit_stop). */
#include "lanec_suites.h"
#include "lanec_pw.h"
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"
#include "agc_transfer.h"
#include <string.h>

enum exec_status lanec_pin_chain(apollo_overlay_state *, lanec_io *, enum lanec_mission, uint16_t stop, uint16_t *steps);

#define AB(o) (LANEC_LM_ABCLOAD + (o))
#define LV LANEC_LM_LOADLV
/* Each point and the entry of the replaced module its states start at. */
static const uint16_t points[][2] = {
    { AB(0), AB(0) }, { AB(2), AB(0) }, { AB(3), AB(0) }, { AB(5), AB(0) }, { AB(6), AB(6) }, { AB(010), AB(6) },
    { AB(011), AB(011) }, { AB(013), AB(011) }, { AB(065), AB(065) }, { AB(067), AB(065) }, { AB(070), AB(065) },
    { AB(072), AB(065) }, { AB(073), AB(073) }, { AB(075), AB(073) }, { AB(0114), AB(0114) }, { AB(0125), AB(0125) },
    { AB(0127), AB(0125) }, { AB(0142), AB(0142) }, { AB(0144), AB(0142) }, { LV, LV }, { LV + 5u, LV },
};
#define POINTS (sizeof points / sizeof points[0])
#define PER 32u
#define CASES (POINTS * 2u * PER)

static int reached(uint32_t n, uint16_t point, uint16_t from, enum lanec_mission mission, apollo_overlay_state *s, lanec_io *io)
{
    const lanec_suite *src = from == LV ? &lanec_suite_pw_load_complete : &lanec_suite_pw_load_request;
    static apollo_overlay_state t;
    static lanec_meta scratch;
    lanec_io u;
    uint32_t k;
    uint16_t steps;
    for (k = 0; k < 4000u; ++k) {
        memset(&scratch, 0, sizeof scratch);
        src->make((n * 977u + k) % src->count, s, io, &scratch);
        if (scratch.mission != mission || s->words[LANEC_Z] != from || lanec_current_bank(s, io) != 041u) continue;
        t = *s; u = *io;
        if (src->run(&t, &u, mission) != LANEC_OK) continue;
        if (point == from) return 1;
        t = *s; u = *io;
        if (lanec_pin_chain(&t, &u, mission, (uint16_t)(041u * 1024u + point - 02000u), &steps) == EXEC_OK &&
            t.words[LANEC_Z] == point && lanec_current_bank(&t, &u) == 041u) {
            *s = t; *io = u;
            return 1;
        }
    }
    return 0;
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t point = points[n % POINTS][0], from = points[n % POINTS][1];
    m->mission = (n / POINTS) & 1u ? LANEC_CM : LANEC_LM;
    if (!reached(n, point, from, m->mission, s, io)) {
        lanec_fill(s, io, n);
        lanec_pw_settle(s);
        lanec_enter(s, io, 041u, point);
    }
    memset(m->range, 0, sizeof m->range);
    m->ranges = 0;
    lanec_pw_exit_stop(m, s, io, lanec_pin_head_load_run);
}
const lanec_suite lanec_suite_pin_head_load = { "pin-head-load", CASES, make, lanec_pin_head_load_run };
