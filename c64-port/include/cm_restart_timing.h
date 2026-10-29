/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_TIMING_H
#define CM_RESTART_TIMING_H
#include "overlay_runtime.h"
/* Source FINDTIME and ITSLGCL2 arithmetic through the transfer to VAC5 GOLOC-1.
 * Input A (or A/L) already contains the directly/indirectly selected delta.
 * Group is zero based. These explicit source boundaries mutate the sole image;
 * caller subsequently executes the selected WAITLIST/LONGCALL native service.
 * Ordinary flight words and sign-extended L are required. Source exclusion is
 * caller-owned; external IRQ/SUPERBNK latches remain unchanged. */
enum cm_restart_time_exit { CM_RSTIME_WAITLIST=0,CM_RSTIME_LONGCALL=1,CM_RSTIME_INVALID=2 };
enum cm_restart_time_exit cm_restart_wait_delay(apollo_overlay_state *,uint8_t group);
enum cm_restart_time_exit cm_restart_long_delay(apollo_overlay_state *);
#endif
