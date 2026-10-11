/* SPDX-License-Identifier: MIT */
#ifndef IMU_ATTITUDE_ADMISSION_CASES_H
#define IMU_ATTITUDE_ADMISSION_CASES_H
#include "imu_attitude_admission.h"
void imu_attitude_admission_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_attitude_stage,uint32_t);
#endif
