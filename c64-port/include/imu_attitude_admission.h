/* SPDX-License-Identifier: MIT */
#ifndef IMU_ATTITUDE_ADMISSION_H
#define IMU_ATTITUDE_ADMISSION_H
#include "imu_pulse.h"
enum imu_attitude_stage {IMU_ATTITUDE_MANEUVER,IMU_ATTITUDE_STATUS,IMU_ATTITUDE_SET_USE,IMU_ATTITUDE_FLAG_RETURN,IMU_ATTITUDE_ALARM_RETURN};
enum imu_attitude_status {IMU_ATTITUDE_POSTJUMP,IMU_ATTITUDE_POODOO,IMU_ATTITUDE_UPFLAG,IMU_ATTITUDE_VARALARM,IMU_ATTITUDE_BANK_RETURN,IMU_ATTITUDE_GOTOPOOH,IMU_ATTITUDE_UNSUPPORTED};
/* Source-local GOMANUR/R02 compiled basic stages, ordinary canonical image
 * and raw A/L, EB3 and mission-specific containing bank, fixed Q, quiescent
 * device activity and excluded producers. Maneuver's saved BUF2 fixed caller
 * and aligned bank are preconditions. Stops actual external helper entry;
 * flag/alarm continuations are explicit compiled stages, not decoded calls. */
enum imu_attitude_status imu_attitude_admission(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_attitude_stage);
uint8_t imu_attitude_literal(enum rtb_mission,uint16_t,agc_word *);
#endif
