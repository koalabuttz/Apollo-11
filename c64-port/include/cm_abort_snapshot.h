/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_SNAPSHOT_H
#define CM_ABORT_SNAPSHOT_H
#include "cm_alarm_record.h"
#include "cm_vac5_store.h"
#include "cm_interbank_extended.h"
/* Source stages:0 actual BAILOUT/POODOO/CCSHOLE through BANKCALL toVAC5;
 * 1 completeVAC5STOR;2 genuineSWRETURN;3 actual abort alarm-code producer to
 * BORTENT;4 source alarm record to caller or genuine overflowPOODOO entry.
 * Inline alarm data is resolved after snapshot return by qualified caller.
 * Recovery continuation is explicit, never synthesized. Producers excluded. */
enum exec_status cm_abort_snapshot_stage(apollo_overlay_state *,cm_native_io *,uint8_t,agc_word original_alarm_word);
#endif
