/* SPDX-License-Identifier: MIT */
/* pw-operator-schedule: Verb 30 VBRQEXEC (41,3466) and Verb 31 VBRQWAIT
 * (41,3512) through RELDSP, REQUESTC and NOVAC/FINDVAC or WAITLIST to
 * ENDOFJOB or the BAILOUT/POODOO alarm (src/dsky_operator_schedule.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM; the request pair is the
 * one the generator stores at 01046/01047. */
#include "lanec_pw_e.h"
#include "dsky_operator_schedule.h"
#include "dsky_operator_schedule_cases.h"

#define CASES 1024u
LANEC_PW_CHARIN_MAKE(make, dsky_operator_schedule_input, lanec_pw_operator_schedule_run, CASES)
const lanec_suite lanec_suite_pw_operator_schedule = { "pw-operator-schedule", CASES, make, lanec_pw_operator_schedule_run };
