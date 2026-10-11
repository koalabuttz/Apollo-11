/* SPDX-License-Identifier: MIT */
#ifndef DSKY_ALARM_RECYCLE_H
#define DSKY_ALARM_RECYCLE_H
#include "dsky_alarm_cycle.h"
#include "dsky_nv_update_general.h"
enum dsky_recycle_route {DSKY_RECYCLE_ENTER,DSKY_RECYCLE_CCS_HOLE};
/* Continuous actual ALMCYCLE through original lamp, verb display and
 * SWRETURN/POSTJUMP to ENTER, or genuine selected-clean+0 CCSHOLE.
 * VERBSAVE is complement of0..99/eitherzero. All source FB/EB, selecting
 * SUPER/IRQ, canonical image/rawCPU; caller holds restoring exclusion. */
enum p66_bias_status dsky_alarm_recycle(apollo_overlay_state *,dsky_charin_io *,enum dsky_recycle_route *);
enum exec_status cm_alarm_recycle(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_recycle_route *);
#endif
