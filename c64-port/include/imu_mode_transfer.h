/* SPDX-License-Identifier: MIT */
#ifndef IMU_MODE_TRANSFER_H
#define IMU_MODE_TRANSFER_H
#include "imu_pulse.h"
enum imu_transfer_stage {IMU_TRANSFER_SEND,IMU_TRANSFER_ENABLE,IMU_TRANSFER_COARSE_ERROR,IMU_TRANSFER_AOT,IMU_TRANSFER_RADAR,IMU_TRANSFER_IMU,IMU_TRANSFER_STALL,IMU_TRANSFER_FAILED,IMU_TRANSFER_GOOD,IMU_TRANSFER_SLEEP,IMU_TRANSFER_ABORT};
enum imu_transfer_status {IMU_TRANSFER_DELAY,IMU_TRANSFER_TASKOVER,IMU_TRANSFER_BAD,IMU_TRANSFER_EXIT,IMU_TRANSFER_JOB_SLEEP,IMU_TRANSFER_BAILOUT,IMU_TRANSFER_UNSUPPORTED};
/* Exclusive original bank07/EB3 compiled basic stages, ordinary RAM/raw A/L,
 * fixed Q, coherent source latches. SEND activates channel14 coarse bits and
 * returns before VARDELAY under +zero CDU command counters and quiescent
 * gyro/coarse activity. Active transport requires its separate device owner. Other bodies
 * retain actual external service entries. STALL A/failed-good device index
 * is0..3; FAILED requires A0, GOOD the nonnegative diminished CCS result.
 * Saved BUF2 fixed caller/bank must be coherent ordinary data. No decoding,
 * scheduling or downstream jobs. All unsupported inputs remain unchanged. */
enum imu_transfer_status imu_mode_transfer(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_transfer_stage);
uint8_t imu_transfer_literal(enum rtb_mission,uint16_t,agc_word *);
#endif
