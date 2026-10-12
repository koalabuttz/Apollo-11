/* SPDX-License-Identifier: MIT */
/* pw-load-complete: LOADLV through RELDSP, POSTJUMP/RECALTST and ENDOFJOB
 * (src/dsky_load_complete.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_load_complete.h"
#include "dsky_load_complete_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_load_complete_input, lanec_pw_lamp_out, lanec_pw_load_complete_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_load_complete = { "pw-load-complete", CASES, make, lanec_pw_load_complete_run };
