/* SPDX-License-Identifier: MIT */
/* pw-charin-sign: CHARIN + and - (keys 032, 033) (src/dsky_charin_sign.c), channel 11 through
 * io->channel[011]. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_a.h"
#include "dsky_charin_sign.h"
#include "dsky_charin_sign_cases.h"
LANEC_PW_CHARIN_RUN(run, dsky_charin_sign(s, &c), cm_charin_sign(s, &x, &w))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_charin_sign_input, 8192u, CASES, run)
const lanec_suite lanec_suite_pw_charin_sign = { "pw-charin-sign", CASES, make, run };
