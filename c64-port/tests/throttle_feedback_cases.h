/* SPDX-License-Identifier: MIT */
#ifndef THROTTLE_FEEDBACK_CASES_H
#define THROTTLE_FEEDBACK_CASES_H
#include "unit_cases.h"
#include "guidance_throttle.h"
#define TF_WORDS 124u
#define TF_CASES 16384u
typedef struct{unit_case previous;p66_throttle_inputs input;p66_throttle_feedback feedback;agc_word bias[6];}throttle_feedback_case;
void tf_input(throttle_feedback_case *,uint16_t);
void tf_words(const throttle_feedback_case *,uint16_t [TF_WORDS]);
#ifndef THROTTLE_FEEDBACK_INPUT_ONLY
unsigned tf_apply(throttle_feedback_case *);
#endif
#endif
