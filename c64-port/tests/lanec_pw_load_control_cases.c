/* SPDX-License-Identifier: MIT */
/* pw-load-control: VBPROC/VBTERM/VBRESEQ/PROCKEY through RECALTST and
 * ENDOFJOB, or direct RECALTST (src/dsky_load_control.c). Generator numbers
 * 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_load_control.h"
#include "dsky_load_control_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_charin_io, dsky_load_control_input, lanec_pw_charin_out, lanec_pw_load_control_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_load_control = { "pw-load-control", CASES, make, lanec_pw_load_control_run };
