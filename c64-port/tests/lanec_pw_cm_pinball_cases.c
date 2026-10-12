/* SPDX-License-Identifier: MIT */
/* pw-cm-pinball: Comanche DSPIN (40,3227), 11DSPIN (40,3311) and 2BLANK
 * (40,2502) to the ordinary caller (src/cm_pinball.c). The generator's
 * number mod 3 picks the entry, which run() reads back from Z. */
#include "lanec_pw_f.h"
#include "cm_pinball_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, CM_PINBALL_CASES);
    lanec_fill(s, io, n);
    cm_pinball_input(s, &c, k);
    while (lanec_pw_f_caller_reentered(s->words[LANEC_Q])) cm_pinball_input(s, &c, ++k);
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_cm_pinball_run);
}
const lanec_suite lanec_suite_pw_cm_pinball = { "pw-cm-pinball", CASES, make, lanec_pw_cm_pinball_run };
