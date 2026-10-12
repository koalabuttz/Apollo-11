/* SPDX-License-Identifier: MIT */
/* pw-charin-clear: CHARIN CLEAR (key 036) (src/dsky_charin_clear.c), channel 11 through
 * io->channel[011]. Generator numbers 0-16383; those with n mod 8192 below 4096 are LM, the rest CM. */
#include "lanec_pw_a.h"
#include "dsky_charin_clear.h"
#include "dsky_charin_clear_cases.h"
LANEC_PW_CHARIN_RUN(run, dsky_charin_clear(s, &c), cm_charin_clear(s, &x, &w))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_charin_clear_input, 16384u, CASES, run)
const lanec_suite lanec_suite_pw_charin_clear = { "pw-charin-clear", CASES, make, run };
