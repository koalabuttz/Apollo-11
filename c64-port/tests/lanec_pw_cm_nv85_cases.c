/* SPDX-License-Identifier: MIT */
/* pw-cm-nv85: the normal V06N85 display route 0-25 (src/cm_nv85_route.c over
 * src/cm_nv85.c): clock job selection, display control, MAKEPLAY and
 * Executive stages 0-6 and 25, the Pinball stages GOPLAY .. finish 7-24.
 * Domain: 512 generator numbers x 26 routes; case k runs route k%26 from
 * the state the native routes 0..k%26-1 leave for generator number k/26
 * (the module's own snapshot chain). make() records the route for run(). */
#include "lanec_pw_f.h"
#include "cm_nv85_cases.h"

static uint8_t route;
LANEC_PW_F_CM_RUN(nv85_run, cm_nv85_step(s, &c, route))
#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint32_t k = lanec_pw_spread(n, CASES, CM_NV85_CASES * 26u);
    uint8_t p;
    lanec_fill(s, io, n);
    cm_nv85_input(s, &c, (uint16_t)(k / 26u));
    for (p = 0; p < k % 26u; ++p)
        if (!cm_nv85_step(s, &c, p)) break;
    route = p;
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, nv85_run);
}
const lanec_suite lanec_suite_pw_cm_nv85 = { "pw-cm-nv85", CASES, make, nv85_run };
