/* SPDX-License-Identifier: MIT */
#ifndef IMU_COARSE_H
#define IMU_COARSE_H
#include "imu_pulse.h"
enum imu_coarse_stage {IMU_COARSE_SET,IMU_COARSE_CALCULATE,IMU_COARSE_SEGMENT,IMU_COARSE_CHECK};
enum imu_coarse_status {IMU_COARSE_RETURN,IMU_COARSE_DELAY,IMU_COARSE_SEND,IMU_COARSE_SETTLE,IMU_COARSE_END,IMU_COARSE_ALARM,IMU_COARSE_BAD,IMU_COARSE_UNSUPPORTED};
/* Actual fixed-source entries. Ordinary image/raw A/L, coherent bank07,
 * EB3 for command words, quiet CDU/gyro producers and signed-zero command counters,
 * CDU activity bits clear; gyro negative-zero requires activity clear,
 * fixed source return Q02000..07777, source latches required. CALCULATE
 * stops at real VARDELAY, SEGMENT at SENDPULS or FIXDELAY, CHECK at ALARM
 * or ENDIMU. SET ends at actual TC Q. External scheduling/transport remains
 * owned by caller; no new poll or opcode decoder. Unsupported unchanged. */
enum imu_coarse_status imu_coarse(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission,enum imu_coarse_stage);
/* Passive original local constants, addressed by physical fixed-word index. */
uint8_t imu_coarse_literal(enum rtb_mission,uint16_t,agc_word *);
#endif
