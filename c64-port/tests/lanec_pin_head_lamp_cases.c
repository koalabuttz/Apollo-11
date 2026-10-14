/* SPDX-License-Identifier: MIT */
/* pin-head-lamp: every point of src/lanec_pin_head_lamp.c, both missions,
 * from the states pw-lamp-lifecycle's accepted cases reach there
 * (lanec_pin_head_reach), else a filled state. The oracle runs the original
 * from the same state to where the head stops (lanec_pw_exit_stop). */
#include <string.h>
#include "lanec_pin_head_cases.h"
#include "lanec_pw.h"
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"

static const struct { enum lanec_mission m; uint16_t point; } points[] = {
    { LANEC_LM, LANEC_LM_VBTSTLTS }, { LANEC_LM, LANEC_LM_TSTLTS1 }, { LANEC_LM, LANEC_LM_TSTLTS2 },
    { LANEC_LM, LANEC_LM_TSTLTS3 }, { LANEC_CM, LANEC_CM_VBTSTLTS }, { LANEC_CM, LANEC_CM_VBTSTLTS + 2u },
    { LANEC_CM, LANEC_CM_TSTLTS1 }, { LANEC_CM, LANEC_CM_TSTLTS2 }, { LANEC_CM, LANEC_CM_TSTLTS3 },
};
#define POINTS (sizeof points / sizeof points[0])
#define PER 64u
#define CASES (POINTS * PER)

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t point = points[n % POINTS].point;
    m->mission = points[n % POINTS].m;
    if (n / POINTS % 4u == 3u ||
        !lanec_pin_head_reach(n, 2000u, &lanec_suite_pw_lamp_lifecycle, 041u, point, m->mission, s, io)) {
        lanec_fill(s, io, n);
        lanec_pw_settle(s);
        lanec_enter(s, io, 041u, point);
        if (point == (m->mission == LANEC_LM ? LANEC_LM_TSTLTS1 : LANEC_CM_TSTLTS1)) s->words[LANEC_A] = (uint16_t)(n / POINTS % 13u);
    }
    memset(m->range, 0, sizeof m->range);
    m->ranges = 0;
    lanec_pw_exit_stop(m, s, io, lanec_pin_head_lamp_run);
}
const lanec_suite lanec_suite_pin_head_lamp = { "pin-head-lamp", CASES, make, lanec_pin_head_lamp_run };
