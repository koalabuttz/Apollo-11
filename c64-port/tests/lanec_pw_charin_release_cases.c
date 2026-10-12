/* SPDX-License-Identifier: MIT */
/* pw-charin-release: CHARIN KEY RELEASE (key 031) (src/dsky_charin_release.c), channel 11 through
 * io->channel[011]. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_a.h"
#include "dsky_charin_release.h"
#include "dsky_charin_release_cases.h"
static enum dsky_release_route route;
LANEC_PW_CHARIN_RUN(run, dsky_charin_release(s, &c, &route), cm_charin_release(s, &x, &w, &route))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_charin_release_input, 8192u, CASES, run)
const lanec_suite lanec_suite_pw_charin_release = { "pw-charin-release", CASES, make, run };
