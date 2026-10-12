/* SPDX-License-Identifier: MIT */
/* pw-charin-alarm: CHARIN for the keys it alarms on (src/dsky_charin_alarm.c), channel 11 through
 * io->channel[011]. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_a.h"
#include "dsky_charin_alarm.h"
#include "dsky_charin_alarm_cases.h"
LANEC_PW_CHARIN_RUN(run, dsky_charin_alarm(s, &c), cm_charin_alarm(s, &x, &w))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_charin_alarm_input, 8192u, CASES, run)
const lanec_suite lanec_suite_pw_charin_alarm = { "pw-charin-alarm", CASES, make, run };
