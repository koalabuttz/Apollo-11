/* SPDX-License-Identifier: MIT */
#ifndef THROTTLE_SERVICE_CASES_H
#define THROTTLE_SERVICE_CASES_H
#include "throttle_feedback_cases.h"
#include "guidance_throttle_service.h"
#define TS_WORDS 147u
#define TS_CASES 16384u
typedef struct{throttle_feedback_case previous;p66_throttle_service_state service;}throttle_service_case;
void ts_input(throttle_service_case *,uint16_t);
void ts_words(const throttle_service_case *,uint16_t [TS_WORDS]);
#ifndef THROTTLE_SERVICE_INPUT_ONLY
unsigned ts_apply(throttle_service_case *);
#endif
#endif
