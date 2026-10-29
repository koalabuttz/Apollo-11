/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_QUICK_CASES_H
#define LM_POWERED_QUICK_CASES_H
#include "overlay_runtime.h"
#include "agc_word.h"
#define LM_POWERED_QUICK_CASES 32768UL
void lm_powered_quick_input(apollo_overlay_state *,agc_word *,uint16_t);
#endif
