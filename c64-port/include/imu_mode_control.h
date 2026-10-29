/* SPDX-License-Identifier: MIT */
#ifndef IMU_MODE_CONTROL_H
#define IMU_MODE_CONTROL_H
#include "imu_coarse.h"
enum imu_mode_stage {IMU_MODE_ZERO_CDU,IMU_MODE_NO_ATT,IMU_MODE_ZERO2,IMU_MODE_ZERO3,IMU_MODE_FAIL_OK,IMU_MODE_PIPA_OK,IMU_MODE_PIPA_USE,IMU_MODE_PIPA_FREE,IMU_MODE_FINE,IMU_MODE_ZERO,IMU_MODE_ZERO_REGISTERED,IMU_MODE_FINE_REGISTERED,IMU_MODE_FINE_DONE,IMU_MODE_WARNING_ZERO_DONE,IMU_MODE_WARNING_FAIL_DONE,IMU_MODE_WARNING_PIPA_DONE,IMU_MODE_PIPA_ALARM_DONE,IMU_MODE_COARSE_START,IMU_MODE_COARSE_REGISTERED,IMU_MODE_ZERO_ALARM_DONE};
enum imu_mode_status {IMU_MODE_RETURN,IMU_MODE_DELAY,IMU_MODE_WARNING_CALL,IMU_MODE_TASKOVER,IMU_MODE_BANK_RETURN,IMU_MODE_EXIT,IMU_MODE_ALARM,IMU_MODE_BAD,IMU_MODE_WAITLIST,IMU_MODE_END,IMU_MODE_UNSUPPORTED};
/* Exclusive canonical basic source stages, bank07/EB3, ordinary RAM/rawA/L,
 * fixedQ and quiet signedzero counters/activity. Actual external helper
 * entries are retained, without executing admitted jobs or pulse delivery.
 * Unsupported is unchanged; all source callbacks retain ownership. */
enum imu_mode_status imu_mode_control(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_mode_stage);
uint8_t imu_mode_literal(enum rtb_mission,uint16_t,agc_word *);
#endif
