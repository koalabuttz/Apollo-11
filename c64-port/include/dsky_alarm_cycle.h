/* SPDX-License-Identifier: MIT */
#ifndef DSKY_ALARM_CYCLE_H
#define DSKY_ALARM_CYCLE_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
enum dsky_alarm_cycle_route {DSKY_ALARM_CYCLE_UPDATE,DSKY_ALARM_CYCLE_ENTER};
/* Actual ALMCYCLE(LM4145/CM4161) through source BANKCALL to UPDATVB-1,
 * or actual SWRETURN/after-call4152/4166 through POSTJUMP to ENTER41,2002.
 * All aligned FB/EB, four selectingSUPER/IRQ, canonical words/rawCPU.
 * SWRETURN uses source-owned literal BUF2 return and aligned saved FB.
 * No formatter/domain substitute; caller holds restoring exclusion. */
enum p66_bias_status dsky_alarm_cycle(apollo_overlay_state *,dsky_charin_io *,enum dsky_alarm_cycle_route *);
enum exec_status cm_alarm_cycle(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_alarm_cycle_route *);
#endif
