/* SPDX-License-Identifier: MIT */
#ifndef LM_BASIC_SLEEP_H
#define LM_BASIC_SLEEP_H
#include "lm_basic_wake.h"
/* Actual JOBSLEEP05133 through selected basic DTCB, or idle ADVAN03214.
 * A sign-extended ordinary sleeping CADR, activepositive core0, normal banks,
 * other core priorities positive/dormant/-0 (not+0). Selectedbasic
 * BBCON supported; SUPER and channel11 explicit, caller owns exclusion.
 * Returns actual next pair or {0,0} for idle; unsupported allobjectsunchanged.
 * Exact canonical scan/debris/channel/IRQ, no decoder. */
enum exec_status lm_basic_sleep(apollo_overlay_state *,p66_bias_io *,agc_word *channel11,exec_address *next);
#endif
