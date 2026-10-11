/* SPDX-License-Identifier: MIT */
#ifndef IMU_PULSE_H
#define IMU_PULSE_H
#include "rtb_vector.h"
typedef struct {p66_bias_io latch;agc_word channel11,channel12,channel14;} imu_pulse_io;
enum imu_pulse_status {IMU_PULSE_WAITLIST=0,IMU_PULSE_SLEEP=1,IMU_PULSE_CAGED_RETURN=2,IMU_PULSE_OUT_OF_SCOPE=3};
/* Original IMUPULSE lead-in, explicit compiled handoff at WAITLIST/JOBSLEEP
 * or complete caged return through MODEEXIT/SWRETURN. No admitted task, sleep
 * selection or gyro output handler runs here. Canonical ordinary image/raw16
 * A/L; source bank07, coherent banks, normalized command ECADR0400..03772
 * with six words within one bank, initialized nonnegative LGYRO (+0 allowed), idle OPTX/OPTY/GYROCTR and three CDU command counters0,
 * fixed saved BUF2 caller and ordinary device latches. Nonnull caller objects
 * and excluded source/command/device producers are required. */
enum imu_pulse_status imu_pulse_enter(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission);
/* Successful original WAITLIST registration at the compiled GWAKE2 caller.
 * Valid nonfull initialized queue and TIME3 are additional preconditions. */
enum imu_pulse_status imu_pulse_register(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission);
/* Source reservation and three TPAGREE exchanges, through MODEEXIT/SWRETURN.
 * Entry follows registered WAITLIST; command is six ordinary E4..E7 words,
 * disjoint from service scratch and the WAITLIST arena. */
enum imu_pulse_status imu_pulse_finish(apollo_overlay_state *,imu_pulse_io *,enum rtb_mission);
#endif
