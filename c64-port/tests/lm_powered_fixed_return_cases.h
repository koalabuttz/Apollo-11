/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_FIXED_RETURN_CASES_H
#define LM_POWERED_FIXED_RETURN_CASES_H
#include "overlay_runtime.h"
#include "agc_word.h"
#define LM_POWERED_FIXED_RETURN_CASES 32768u
void lm_powered_fixed_return_input(apollo_overlay_state *,agc_word *,uint16_t,uint8_t);
#endif
