/* SPDX-License-Identifier: MIT */
#ifndef CM_DIFEQ_H
#define CM_DIFEQ_H
#include "overlay_runtime.h"
/* Source Nystrom integration contributions DIFEQ+0/+1 with DIFEQCOM, and
 * DIFEQ+2 through STORE ZV. Explicit native source phases; no decoder.
 * Caller owns the canonical physical image and excludes other producers.
 * Each genuine DANZIG return allows the original Executive job check. */
enum cm_difeq_status {CM_DIFEQ_OK=0,CM_DIFEQ_OUT_OF_SCOPE=1};
enum cm_difeq_status cm_difeq_phase(apollo_overlay_state *,uint8_t stage,uint8_t phase);
uint8_t cm_difeq_phase_count(uint8_t stage);
#endif
