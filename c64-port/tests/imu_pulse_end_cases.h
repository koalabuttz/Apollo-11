/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_END_CASES_H
#define IMU_PULSE_END_CASES_H
#include "imu_pulse_end.h"
#define IMU_PULSE_END_CASES 65536u
void imu_pulse_end_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_gyro_end_entry,uint32_t);
#endif
