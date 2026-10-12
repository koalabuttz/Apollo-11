/* SPDX-License-Identifier: MIT */
/* pw-alarm-recycle: the alarm RECYCLE path through the alarm cycle and the NV update (src/dsky_alarm_recycle.c), channel 11 through
 * io->channel[011]. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_a.h"
#include "dsky_alarm_recycle.h"
#include "dsky_alarm_recycle_cases.h"
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_alarm_recycle_input, 8192u, CASES, lanec_pw_alarm_recycle_run)
const lanec_suite lanec_suite_pw_alarm_recycle = { "pw-alarm-recycle", CASES, make, lanec_pw_alarm_recycle_run };
