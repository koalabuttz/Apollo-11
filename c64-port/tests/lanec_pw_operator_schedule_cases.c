/* SPDX-License-Identifier: MIT */
/* pw-operator-schedule: Verb 30 VBRQEXEC (41,3466) and Verb 31 VBRQWAIT
 * (41,3512) through RELDSP, REQUESTC and NOVAC/FINDVAC or WAITLIST to
 * ENDOFJOB or the BAILOUT/POODOO alarm (src/dsky_operator_schedule.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM; the request pair is the
 * one the generator stores at 01046/01047. */
#include "lanec_pw_e.h"
#include "dsky_operator_schedule.h"
#include "dsky_operator_schedule_cases.h"

static exec_address request_pair(const apollo_overlay_state *s)
{
    exec_address a;
    a.location = s->words[01046]; a.bank = s->words[01047];
    return a;
}
static enum dsky_operator_schedule_route route;
LANEC_PW_CHARIN_RUN(operator_schedule_run, dsky_operator_schedule(s, &p, request_pair(s), &route),
                    cm_operator_schedule(s, &c, &wrote, request_pair(s), &route))
#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_operator_schedule_input, operator_schedule_run, CASES)
const lanec_suite lanec_suite_pw_operator_schedule = { "pw-operator-schedule", CASES, make, operator_schedule_run };
