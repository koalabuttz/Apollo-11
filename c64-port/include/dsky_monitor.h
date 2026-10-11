/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MONITOR_H
#define DSKY_MONITOR_H
#include "dsky_charin_nv.h"
#include "dsky_nv_admission.h"
enum dsky_monitor_stage {DSKY_MONITOR_SETUP,DSKY_MONITOR_CYCLE,DSKY_MONITOR_CYCLE_RAW,DSKY_MONITOR_PUBLISH,DSKY_MONITOR_REQUEST,DSKY_MONITOR_REQUEST_REGISTERED,DSKY_MONITOR_REQUEST_ADMITTED,DSKY_MONITOR_KILL,DSKY_MONITOR_DISPLAY,DSKY_MONITOR_DISPLAY_AFTER,DSKY_MONITOR_BUSY};
enum dsky_monitor_route {DSKY_MONITOR_RELEASE,DSKY_MONITOR_CYCLE_ENTRY,DSKY_MONITOR_RAW_ENTRY,DSKY_MONITOR_ENTRET,DSKY_MONITOR_WAITLIST,DSKY_MONITOR_NOVAC,DSKY_MONITOR_TASKOVER,DSKY_MONITOR_UPDATE,DSKY_MONITOR_END,DSKY_MONITOR_BUSY_ENTRY,DSKY_MONITOR_TESTNN,DSKY_MONITOR_REQUEST_ENTRY};
/* Original MONITOR/MONREQ/KILLMON/MONDO/MONBUSY local compiled bodies;
 * actual release/admission/update/TESTNN/ENTRET/END continuations remain
 * explicit caller-owned entries. REQUEST includes real LODSAMPT copy.
 * Bank41/allEB/selectingSUPER, ordinary RAM/rawA/L, channel11/IRQ, distinct
 * canonical IO/route and restoring producer exclusion. Unsupported no-op.
 * No instruction decoder, wallclock, admitted task or scheduler body. */
enum p66_bias_status dsky_monitor(apollo_overlay_state *,dsky_charin_io *,enum dsky_nv_mission,enum dsky_monitor_stage,enum dsky_monitor_route *);
/* Passive original constants/inline admission tuple data, physical fixed word. */
uint8_t dsky_monitor_literal(enum dsky_nv_mission,uint16_t,agc_word *);
#endif
