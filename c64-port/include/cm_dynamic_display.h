/* SPDX-License-Identifier: MIT */
#ifndef CM_DYNAMIC_DISPLAY_H
#define CM_DYNAMIC_DISPLAY_H
#include "cm_native_image.h"
/* Actual admitted DYNDISP24,3325 -> normal V06N85 -> ENDOFJOB/idle.
 * Compiled native route0..52; NEWJOB+0, active allocated VAC, no other job,
 * normal unblocked display. Caller owns image/exclusion/overlay routing.
 * Each linked call must return before its code overlay is replaced. */
uint8_t cm_dynamic_display_step(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
