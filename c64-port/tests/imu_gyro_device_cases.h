/* SPDX-License-Identifier: MIT */
#ifndef IMU_GYRO_DEVICE_CASES_H
#define IMU_GYRO_DEVICE_CASES_H
#include "imu_gyro_device.h"
#define IMU_GYRO_DEVICE_CASES 65536UL
void imu_gyro_device_input(apollo_overlay_state *,imu_pulse_io *,imu_gyro_device *,enum rtb_mission,enum imu_gyro_clock_mode,uint32_t);
#endif
