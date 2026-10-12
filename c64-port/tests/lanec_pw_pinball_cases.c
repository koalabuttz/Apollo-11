/* SPDX-License-Identifier: MIT */
/* pw-pinball: Luminary DSPIN (40,3322), 11DSPIN (40,3404) and 2BLANK (40,2601) to
 * the caller (src/dsky_pinball.c), the LM side of pw-cm-pinball; the entry
 * is read back from Z. */
#include "lanec_pw_g.h"
#include "dsky_pinball.h"
#include "dsky_pinball_cases.h"

#define CASES 1024u
LANEC_PW_G_MAKE(make, dsky_pinball_prepare, DSKY_PINBALL_CASE_COUNT, CASES, lanec_pw_pinball_run, io->channel[011])
const lanec_suite lanec_suite_pw_pinball = { "pw-pinball", CASES, make, lanec_pw_pinball_run };
