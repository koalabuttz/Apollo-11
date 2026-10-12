/* SPDX-License-Identifier: MIT */
/* pw-jam-control: fixed-fixed JAMPROC/JAMTERM through the SUPER write,
 * POSTJUMP and VBPROC/VBTERM, RECALTST and ENDOFJOB (src/dsky_jam_control.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_jam_control.h"
#include "dsky_jam_control_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_charin_io, dsky_jam_control_input, lanec_pw_charin_out, lanec_pw_jam_control_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_jam_control = { "pw-jam-control", CASES, make, lanec_pw_jam_control_run };
