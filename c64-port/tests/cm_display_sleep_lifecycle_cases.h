/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPLAY_SLEEP_LIFECYCLE_CASES_H
#define CM_DISPLAY_SLEEP_LIFECYCLE_CASES_H
#include "cm_makeplay_sleep.h"
#include "cm_basic_sleep.h"
#include "cm_basic_wake.h"
#define CM_DISPLAY_SLEEP_LIFECYCLE_CASES 512u
void cm_display_sleep_lifecycle_input(apollo_overlay_state *,cm_native_io *,uint16_t);
uint16_t cm_display_sleep_lifecycle_apply(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
