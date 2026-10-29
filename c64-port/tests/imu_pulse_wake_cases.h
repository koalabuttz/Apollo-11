/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_WAKE_CASES_H
#define IMU_PULSE_WAKE_CASES_H
#include "imu_pulse_wake.h"
#define IMU_PULSE_WAKE_CASES 65536UL
void imu_pulse_wake_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,uint32_t);
#endif
