/* SPDX-License-Identifier: MIT */
#ifndef CM_RECTIFY_CASES_H
#define CM_RECTIFY_CASES_H
#include "cm_rectify.h"
#define CM_RECTIFY_CASES 2048u
#define CM_RECTIFY_PHASES 12u
void cm_rectify_input(apollo_overlay_state *,uint16_t n);
uint8_t cm_rectify_first(uint16_t n);
#endif
