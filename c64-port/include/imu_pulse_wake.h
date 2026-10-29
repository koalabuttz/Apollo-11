/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_WAKE_H
#define IMU_PULSE_WAKE_H
#include "imu_pulse.h"
enum imu_gyro_wake_status {IMU_GYRO_WAKE_WAITLIST=0,IMU_GYRO_WAKE_SLEEP=1,IMU_GYRO_WAKE_TRANSFER=2,IMU_GYRO_WAKE_IDLE=3,IMU_GYRO_WAKE_UNSUPPORTED=4};
/* Actual GWAKE after source basic-job restoration; initialized LGYRO>=+0,
 * source bank07 and exclusively owned ordinary canonical RAM/I/O. Free gyros
 * restore original BUF2 and enter WAITLIST; busy gyros retain saved commands
 * and enter JOBSLEEP. No scheduling check at these compiled ABI boundaries.
 * helper composes frozen source WAITLIST/basic-sleep services; selected job,
 * idle ADVAN and free WAITLIST-return are explicit boundaries. Nonnull caller
 * owned objects, quiescent counters, valid helper Executive/queue domains. */
enum imu_gyro_wake_status imu_gyro_wake(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission);
enum imu_gyro_wake_status imu_gyro_wake_helper(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission);
#endif
