/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_INTERFACE_CASES_H
#define LM_POWERED_INTERFACE_CASES_H
#include "overlay_runtime.h"
#include "agc_word.h"
#define LM_POWERED_INTERFACE_CASES 8192UL
void lm_powered_interface_input(apollo_overlay_state *,agc_word *,uint16_t n,uint8_t selector);
#endif
