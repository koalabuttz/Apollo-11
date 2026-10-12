/* SPDX-License-Identifier: MIT */
/* pw-source-alarm: GODSPALM/PREDSPAL/DSPALARM/CHARALRM/ALM-END and the fixed
 * FALTON, FALTOF/RELDSPON lamp bodies (src/dsky_source_alarm.c). Generator
 * numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_e.h"
#include "dsky_source_alarm.h"
#include "dsky_source_alarm_cases.h"

#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_source_alarm_input, lanec_pw_source_alarm_run, CASES)
const lanec_suite lanec_suite_pw_source_alarm = { "pw-source-alarm", CASES, make, lanec_pw_source_alarm_run };
