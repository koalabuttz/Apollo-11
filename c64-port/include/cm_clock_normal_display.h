/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCK_NORMAL_DISPLAY_H
#define CM_CLOCK_NORMAL_DISPLAY_H
#include "cm_clock_display_mission.h"
/* Genuine CLOCKJOB normal GOPLAY consumer, compiled phases7..29. Uses
 * authoritative mission image/IO, preserving suspended cores. Final29 runs
 * actual ENDOFJOB and records source-selected basic/interpreter/idle transfer.
 * Caller owns exclusion; all calls return before overlay replacement. */
enum cm_service_status cm_clock_normal_display_step(cm_mission_loop *,uint8_t);
#endif
