/* SPDX-License-Identifier: MIT */
#ifndef IMU_STANDBY_CASES_H
#define IMU_STANDBY_CASES_H
#include "imu_standby.h"
void imu_standby_input(apollo_overlay_state *,imu_standby_io *,enum rtb_mission,enum imu_standby_stage,uint32_t);
#endif
