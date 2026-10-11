/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCK_BLOCKED_DISPLAY_H
#define CM_CLOCK_BLOCKED_DISPLAY_H
#include "cm_clock_display_mission.h"
/* Genuine CLOCKJOB blocked GOPLAY path, compiled phases7..11:
 * caller→NVMONOPT, original blocked return, REST, NVSUBUSY admission,
 * actual JOBSLEEP with source-selected transfer. Canonical image retained;
 * source external-monitor flag is never cleared/replaced by a factory. */
enum cm_service_status cm_clock_blocked_display_step(cm_mission_loop *,uint8_t);
#endif
