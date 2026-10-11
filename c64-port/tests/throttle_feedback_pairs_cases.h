/* SPDX-License-Identifier: MIT */
#ifndef THROTTLE_FEEDBACK_PAIRS_CASES_H
#define THROTTLE_FEEDBACK_PAIRS_CASES_H
#include "throttle_feedback_cases.h"
#include "guidance_feedback_pairs.h"
#define TFP_WORDS (TF_WORDS+13u)
typedef struct{throttle_feedback_case source;p66_feedback_pair_control control;}throttle_feedback_pairs_case;
uint8_t tfp_next(agc_word loc);
void tfp_input(throttle_feedback_pairs_case *,uint16_t);
void tfp_words(const throttle_feedback_pairs_case *,uint16_t [TFP_WORDS]);
unsigned tfp_apply(throttle_feedback_pairs_case *);
#endif
