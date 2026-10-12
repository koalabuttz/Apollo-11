/* SPDX-License-Identifier: MIT */
/* pw-response-recall: the response recall entries (src/dsky_response_recall.c).
 * Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_e.h"
#include "dsky_response_recall.h"
#include "dsky_response_recall_cases.h"

LANEC_PW_LAMP_RUN(response_recall_run, dsky_response_recall(s, &p),
                  cm_response_recall(s, &c, &wrote, &restart))
#define CASES 1024u
LANEC_PW_LAMP_MAKE(make, dsky_response_recall_input, response_recall_run, CASES)
const lanec_suite lanec_suite_pw_response_recall = { "pw-response-recall", CASES, make, response_recall_run };
