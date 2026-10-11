/* SPDX-License-Identifier: MIT */
#ifndef CM_POWERED_TRIG_CASES_H
#define CM_POWERED_TRIG_CASES_H
#include "overlay_runtime.h"
#include "agc_word.h"
#define CM_POWERED_TRIG_CASES 32768UL
void cm_powered_trig_input(apollo_overlay_state *,agc_word *,uint16_t n,uint8_t live);
#endif
