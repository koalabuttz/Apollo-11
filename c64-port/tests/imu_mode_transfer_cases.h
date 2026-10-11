/* SPDX-License-Identifier: MIT */
#ifndef IMU_MODE_TRANSFER_CASES_H
#define IMU_MODE_TRANSFER_CASES_H
#include "imu_mode_transfer.h"
void imu_mode_transfer_input(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_transfer_stage,uint32_t);
#endif
