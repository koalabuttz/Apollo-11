/* SPDX-License-Identifier: MIT */
#ifndef IMU_MODE_CONTROL_CASES_H
#define IMU_MODE_CONTROL_CASES_H
#include "imu_mode_control.h"
void imu_mode_control_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_mode_stage,uint32_t);
#endif
