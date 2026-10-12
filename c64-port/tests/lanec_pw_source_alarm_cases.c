/* SPDX-License-Identifier: MIT */
/* pw-source-alarm: GODSPALM/PREDSPAL/DSPALARM/CHARALRM/ALM-END and the fixed
 * FALTON, FALTOF/RELDSPON lamp bodies (src/dsky_source_alarm.c). Generator
 * numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_e.h"
#include "dsky_source_alarm.h"
#include "dsky_source_alarm_cases.h"

static enum dsky_alarm_route route;
LANEC_PW_CHARIN_RUN(source_alarm_run, dsky_source_alarm(s, &p, &route),
                    cm_source_alarm(s, &c, &wrote, &route))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_source_alarm_input, source_alarm_run, CASES)
const lanec_suite lanec_suite_pw_source_alarm = { "pw-source-alarm", CASES, make, source_alarm_run };
