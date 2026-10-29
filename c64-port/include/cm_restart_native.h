/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_NATIVE_H
#define CM_RESTART_NATIVE_H
#include "cm_restart.h"
#include "cm_native_dispatch.h"
/* Source-selected positive Type C JOBs, groups six through one. Resident
 * mapping/callback tables must remain alive. Caller owns producer exclusion
 * and distinct transient views. This stages source admission; START and STEP
 * call the existing trampoline, so selected native C callbacks really execute
 * and return before any exchange. No pending overlay frame may cross STEP.
 * Unsupported mappings/types fail before cleanup. Valid source resource alarm
 * effects remain visible. Channels and display continuation are caller work. */
enum exec_status cm_restart_native_stage(cm_exec_native *,cm_restart_state *,
 cm_waitlist *,const cm_restart_phases *,uint8_t initialize_counters);
/* Actual NUDIRECT/NUCHANG2 channel11 WOR readback, including source LOC
 * debris for a slot-zero start. Existing primitive assumes channel11=0. */
enum exec_status cm_restart_native_start(cm_exec_native *,cm_restart_state *);
#endif
