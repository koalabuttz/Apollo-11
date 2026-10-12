/* SPDX-License-Identifier: MIT */
/* pw-cm-decimal-pair: Comanche DSPDC2NR (40,3173) to the ordinary caller
 * (src/cm_decimal_pair.c), the generator's whole 1024-number domain. */
#include "lanec_pw_f.h"
#include "cm_decimal_pair_cases.h"

LANEC_PW_F_CM_RUN(decimal_pair_run, cm_decimal_pair(s, &c) == EXEC_OK)
#define CASES CM_DECIMAL_PAIR_CASES
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, CM_DECIMAL_PAIR_CASES);
    lanec_fill(s, io, n);
    cm_decimal_pair_input(s, &c, k);
    while (lanec_pw_f_caller_reentered(s->words[LANEC_Q])) cm_decimal_pair_input(s, &c, ++k);
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, decimal_pair_run);
}
const lanec_suite lanec_suite_pw_cm_decimal_pair = { "pw-cm-decimal-pair", CASES, make, decimal_pair_run };
