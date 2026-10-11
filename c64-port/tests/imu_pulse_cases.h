/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_CASES_H
#define IMU_PULSE_CASES_H
#include "imu_pulse.h"
#define IMU_PULSE_CASES 65536u
void imu_pulse_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,uint32_t);
void imu_pulse_service_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,uint32_t);
#endif
