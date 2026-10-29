/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_TASK_CASES_H
#define IMU_PULSE_TASK_CASES_H
#include "imu_pulse_task.h"
#define IMU_PULSE_TASK_CASES 65536u
void imu_pulse_task_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,uint8_t,uint32_t);
#endif
