/* SPDX-License-Identifier: MIT */
#ifndef CM_MAKEPLAY_SLEEP_CASES_H
#define CM_MAKEPLAY_SLEEP_CASES_H
#include "cm_makeplay_sleep.h"
#define CM_MAKEPLAY_SLEEP_CASES 6144u
void cm_makeplay_sleep_input(apollo_overlay_state *,cm_native_io *,uint16_t);
enum cm_display_sleep_boundary cm_makeplay_sleep_apply(apollo_overlay_state *,cm_native_io *,uint16_t);
#endif
