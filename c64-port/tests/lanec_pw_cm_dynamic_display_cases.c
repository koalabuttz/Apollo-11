/* SPDX-License-Identifier: MIT */
/* pw-cm-dynamic-display: DYNDISP (24,3325) -> normal V06N85 -> ENDOFJOB,
 * route 0-52 (src/cm_dynamic_display.c: routes 0-26 are cm_dyndisp_step,
 * 27-52 the V06N85 route 0-25). Domain: 64 generator numbers x 53 routes;
 * case k runs route k%53 from the state the native routes 0..k%53-1 leave
 * for generator number k/53. run() tries each route
 * (LANEC_PW_F_CM_STEPS). */
#include "lanec_pw_f.h"
#include "cm_dynamic_display_cases.h"

#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint32_t k = lanec_pw_spread(n, CASES, CM_DYNAMIC_DISPLAY_CASES * 53u);
    uint8_t p;
    lanec_fill(s, io, n);
    cm_dynamic_display_input(s, &c, (uint16_t)(k / 53u));
    for (p = 0; p < k % 53u; ++p)
        if (!cm_dynamic_display_step(s, &c, p)) break;
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_f_exit_stop(m, s, io, lanec_pw_cm_dynamic_display_run);
}
const lanec_suite lanec_suite_pw_cm_dynamic_display = { "pw-cm-dynamic-display", CASES, make, lanec_pw_cm_dynamic_display_run };
