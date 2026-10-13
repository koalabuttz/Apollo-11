/* SPDX-License-Identifier: MIT */
/* pw-cm-minutes-seconds: Comanche M/SOUT (42,3216) phases 0-5
 * (src/cm_minutes_seconds.c). Domain: 512 generator numbers x 6 phases;
 * case k starts phase k%6 from the state the native phases 0..k%6-1 leave
 * for generator number k/6 (the module's own snapshot chain). run() tries each
 * phase (LANEC_PW_F_CM_STEPS). */
#include "lanec_pw_f.h"
#include "cm_minutes_seconds_cases.h"

#define CASES 2048u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint32_t k = lanec_pw_spread(n, CASES, CM_MINUTES_SECONDS_CASES * 6u);
    uint8_t p;
    lanec_fill(s, io, n);
    cm_minutes_seconds_input(s, &c, (uint16_t)(k / 6u));
    for (p = 0; p < k % 6u; ++p)
        if (!cm_minutes_seconds_phase(s, &c, p)) break;
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_cm_minutes_seconds_run);
}
const lanec_suite lanec_suite_pw_cm_minutes_seconds = { "pw-cm-minutes-seconds", CASES, make, lanec_pw_cm_minutes_seconds_run };
