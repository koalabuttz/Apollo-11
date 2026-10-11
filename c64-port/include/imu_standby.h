/* SPDX-License-Identifier: MIT */
#ifndef IMU_STANDBY_H
#define IMU_STANDBY_H
#include "imu_pulse.h"
typedef struct {imu_pulse_io pulse;agc_word channel13;} imu_standby_io;
enum imu_standby_stage {IMU_STANDBY_SET_FLAG,IMU_STANDBY_PRE_BEGIN,IMU_STANDBY_PRE_RETRY,IMU_STANDBY_PRE_READY,IMU_STANDBY_PRE_REF_RETURN,IMU_STANDBY_PRE_USE_RETURN,IMU_STANDBY_PRE_RENDEZ_RETURN,IMU_STANDBY_PRE_PHASE_RETURN,IMU_STANDBY_PRE_TERMINATE,IMU_STANDBY_PRE_PROCEED,IMU_STANDBY_PRE_RECYCLE,IMU_STANDBY_SCAL_BEGIN,IMU_STANDBY_SCAL_FINISH,IMU_STANDBY_POST_BEGIN,IMU_STANDBY_POST_RETRY,IMU_STANDBY_POST_READY,IMU_STANDBY_POST_SHORT_RETURN,IMU_STANDBY_POST_AGREE_RETURN,IMU_STANDBY_POSTCOM,IMU_STANDBY_POST_TIME_RETURN,IMU_STANDBY_POST_FLAG_RETURN};
enum imu_standby_status {IMU_STANDBY_UPFLAG,IMU_STANDBY_SCALPREP,IMU_STANDBY_PRESTAND,IMU_STANDBY_BANKCALL,IMU_STANDBY_DOWNFLAG,IMU_STANDBY_PHASE,IMU_STANDBY_PROMPT,IMU_STANDBY_FINETIME,IMU_STANDBY_RETRY,IMU_STANDBY_READY,IMU_STANDBY_SHORTMP,IMU_STANDBY_TPAGREE,IMU_STANDBY_GOTOPOOH,IMU_STANDBY_UNSUPPORTED};
/* Original local standby stages and actual typed external callee boundaries.
 * Ordinary source RAM/raw A/L, coherent containing bank/all EB, quiescent
 * device producers/IRQ/SUPER (LM bank37 requires0100 clear)/channels (channel13 unreadable trap-enable
 * bits034000 clear); caller keeps all producers excluded.
 * SCALPREP accepts only its two original compiled callers. FINISH consumes
 * the actual FINETIME ABI (positive14-bit A/L, excluded low-scaler sentinel)
 * and executes only their compiled RAM return, without opcode decoding.
 * Helper-return stages require their actual source Q/L/caller-frame words.
 * No scaler sampling, external services, standby transition or job runs here.
 * Unsupported inputs are unchanged; internal returns retain source ownership. */
enum imu_standby_status imu_standby(apollo_overlay_state *,imu_standby_io *,enum rtb_mission,enum imu_standby_stage);
uint8_t imu_standby_literal(enum rtb_mission,uint16_t,agc_word *);
#endif
