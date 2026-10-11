/* SPDX-License-Identifier: MIT */
#ifndef GIMBAL_CASES_H
#define GIMBAL_CASES_H
#include "gimbal_stage.h"
#define GIMBAL_CASES_PER_MISSION 1280u
#define GIMBAL_CASE_COUNT 2560u
void gimbal_prepare(apollo_overlay_state *,gimbal_io *,uint16_t);
enum apollo_fixed_mission gimbal_mission(uint16_t);
#endif
