/* SPDX-License-Identifier: MIT */
#ifndef CM_TRIG_CASES_H
#define CM_TRIG_CASES_H
#include "cm_quictrig.h"
#define CM_SINGLE_TRIG_CASES UINT32_C(65536)
#define CM_QUIC_TRIG_CASES UINT32_C(33024)
uint32_t cm_trig_count(uint8_t selector);
void cm_trig_input(apollo_overlay_state *,agc_word *,uint16_t n,uint8_t selector);
#endif
