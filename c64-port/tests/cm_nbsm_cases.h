/* SPDX-License-Identifier: MIT */
#ifndef CM_NBSM_CASES_H
#define CM_NBSM_CASES_H
#include "overlay_runtime.h"
#include "agc_word.h"
#define CM_NBSM_CASES 8192UL
void cm_nbsm_input(apollo_overlay_state *,agc_word *,uint16_t);
#endif
