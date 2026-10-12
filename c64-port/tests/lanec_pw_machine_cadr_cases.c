/* SPDX-License-Identifier: MIT */
/* pw-machine-cadr: INTMCTBS through VERBFAN or the selected CCSHOLE
 * (src/dsky_machine_cadr.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_machine_cadr.h"
#include "dsky_machine_cadr_cases.h"

LANEC_PW_ROUTE_RUN(machine_cadr_run, dsky_machine_cadr, cm_machine_cadr, enum dsky_machine_route, DSKY_MACHINE_VERB_FAN)
#define CASES 1024u
LANEC_PW_MAKE(make, p66_bias_io, dsky_machine_cadr_input, lanec_pw_copy_p66, machine_cadr_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_machine_cadr = { "pw-machine-cadr", CASES, make, machine_cadr_run };
