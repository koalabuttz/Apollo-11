/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_TRIG_CASES_H
#define LM_POWERED_TRIG_CASES_H
#include "overlay_runtime.h"
#include "agc_word.h"
#define LM_POWERED_TRIG_CASES 32768UL
void lm_powered_trig_input(apollo_overlay_state *,agc_word *,uint16_t n,uint8_t live);
#endif
