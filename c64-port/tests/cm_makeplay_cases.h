/* SPDX-License-Identifier: MIT */
#ifndef CM_MAKEPLAY_CASES_H
#define CM_MAKEPLAY_CASES_H
#include "cm_makeplay.h"
#define CM_MAKEPLAY_CASES 24576u
void cm_makeplay_input(apollo_overlay_state *,cm_native_io *,uint16_t);
enum cm_makeplay_boundary cm_makeplay_apply(apollo_overlay_state *,cm_native_io *,uint16_t);
#endif
