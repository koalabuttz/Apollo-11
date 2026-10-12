/* SPDX-License-Identifier: MIT */
/* pw-alarm-recycle: the alarm RECYCLE path through the alarm cycle and the NV update (src/dsky_alarm_recycle.c), channel 11 through
 * io->channel[011]. Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_a.h"
#include "dsky_alarm_recycle.h"
#include "dsky_alarm_recycle_cases.h"
static enum dsky_recycle_route route;
LANEC_PW_CHARIN_RUN(run, dsky_alarm_recycle(s, &c, &route), cm_alarm_recycle(s, &x, &w, &route))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_alarm_recycle_input, 8192u, CASES, run)
const lanec_suite lanec_suite_pw_alarm_recycle = { "pw-alarm-recycle", CASES, make, run };
