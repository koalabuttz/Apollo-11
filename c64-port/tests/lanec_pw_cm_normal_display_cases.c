/* SPDX-License-Identifier: MIT */
/* pw-cm-normal-display: the normal verb 6 noun 40 / noun 85 display route
 * (src/cm_normal_display_route.c over src/cm_normal_display.c): routes 0-29
 * for generator numbers 0-511 (N40, M/SOUT included), 0-25 for 512-639
 * (N85). Domain: 512 x 30 + 128 x 26 = 18688 (number, route) pairs; each case
 * runs its route from the state the native routes before it leave (the
 * module's own snapshot chain). run() tries each route
 * (LANEC_PW_F_CM_STEPS). */
#include "lanec_pw_f.h"
#include "cm_normal_display_cases.h"

#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint32_t k = lanec_pw_spread(n, CASES, 512u * 30u + 128u * 26u), number, last;
    uint8_t p;
    if (k < 512u * 30u) { number = k / 30u; last = k % 30u; }
    else { number = 512u + (k - 512u * 30u) / 26u; last = (k - 512u * 30u) % 26u; }
    lanec_fill(s, io, n);
    cm_normal_display_input(s, &c, (uint16_t)number);
    for (p = 0; p < last; ++p)
        if (!cm_normal_display_step(s, &c, p)) break;
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_cm_normal_display_run);
}
const lanec_suite lanec_suite_pw_cm_normal_display = { "pw-cm-normal-display", CASES, make, lanec_pw_cm_normal_display_run };
