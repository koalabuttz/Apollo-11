/* SPDX-License-Identifier: MIT */
#ifndef DSKY_OPERATOR_SCHEDULE_H
#define DSKY_OPERATOR_SCHEDULE_H
#include "dsky_operator_request.h"
#include "lm_admission_image.h"
#include "lm_waitlist_image.h"
#include "cm_admission_complete.h"
#include "cm_waitlist_extended.h"
enum dsky_operator_schedule_route {DSKY_SCHEDULE_END,DSKY_SCHEDULE_ADMISSION_ALARM,DSKY_SCHEDULE_WAIT_FULL,DSKY_SCHEDULE_WAIT_BAD};
/* Actual operator V30/V31 through real RELDSP, scheduler Q0155 inline
 * rereads and TCENDO at MPAC+3 to END or actual source recovery entry.
 * Compiled request witness equals noun26 components2/3; no rope fetch.
 * Packed nonzero normal priorities; positive wait delays<=16250 or ordinary
 * zero/negative sourcebad-delay. Canonical scheduler/wake resource domains.
 * Caller retains restoring exclusion. Recovery/END are true continuations. */
enum p66_bias_status dsky_operator_schedule(apollo_overlay_state *,dsky_charin_io *,exec_address,enum dsky_operator_schedule_route *);
enum exec_status cm_operator_schedule(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,exec_address,enum dsky_operator_schedule_route *);
#endif
