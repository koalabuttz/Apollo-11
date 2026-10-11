/* SPDX-License-Identifier: MIT */
#ifndef GIMBAL_COMPLETE_CASES_H
#define GIMBAL_COMPLETE_CASES_H
#include "gimbal_complete.h"
#define GIMBAL_COMPLETE_CASE_COUNT 10240u
void gimbal_complete_prepare(apollo_overlay_state *,gimbal_io *,uint16_t);
enum apollo_fixed_mission gimbal_complete_mission(uint16_t);
#endif
