/* SPDX-License-Identifier: MIT */
#ifndef VELOCITY_PHASED_CASES_H
#define VELOCITY_PHASED_CASES_H
#include "velocity_cases.h"
#include "guidance_velocity_phased.h"
#define VLP_WORDS (VL_WORD_COUNT+1u)
typedef struct {velocity_case previous;agc_word addrwd;} velocity_phased_case;
void vlp_words(const velocity_phased_case *,uint16_t [VLP_WORDS]);
void vlp_from_words(velocity_phased_case *,const uint16_t [VLP_WORDS]);
#ifndef VELOCITY_PHASED_INPUT_ONLY
unsigned vlp_apply(velocity_phased_case *,uint8_t);
#endif
#endif
