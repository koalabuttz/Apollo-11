/* SPDX-License-Identifier: MIT */
/* pw-cm-formatter: Comanche DSPDECVN (40,3213), DSPDECWD (40,3133, rounded)
 * and DSPDECNR (40,3167) (src/cm_formatter.c). Generator numbers 0-4095 are
 * DSPDECVN, 4096-8191 DSPDECWD, 8192-12287 DSPDECNR; run() picks the routine
 * from Z. */
#include "lanec_pw_f.h"
#include "cm_formatter_cases.h"

#define CASES 1536u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, CM_FORMATTER_CASES);
    lanec_fill(s, io, n);
    cm_formatter_input(s, &c, k);
    while (lanec_pw_f_caller_reentered(s->words[LANEC_Q])) cm_formatter_input(s, &c, ++k);
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_cm_formatter_run);
}
const lanec_suite lanec_suite_pw_cm_formatter = { "pw-cm-formatter", CASES, make, lanec_pw_cm_formatter_run };
