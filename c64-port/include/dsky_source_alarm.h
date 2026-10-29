/* SPDX-License-Identifier: MIT */
#ifndef DSKY_SOURCE_ALARM_H
#define DSKY_SOURCE_ALARM_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
enum dsky_alarm_route {DSKY_ALARM_CALLER,DSKY_ALARM_END,DSKY_ALARM_PASTE,DSKY_ALARM_ABORT,DSKY_ALARM_PINBRANCH};
/* Actual GODSPALM/PREDSPAL/DSPALARM/CHARALRM/ALM-END and fixed FALTON,
 * FALTOF/RELDSPON. Source-selected END, PASTEVB, POODOO or PINBRNCH entry;
 * fixed lamp bodies return ordinary Q caller0100..7777. Canonical image,
 * explicit channel11 event, caller restoring producer exclusion. */
enum p66_bias_status dsky_source_alarm(apollo_overlay_state *,dsky_charin_io *,enum dsky_alarm_route *);
enum exec_status cm_source_alarm(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_alarm_route *);
#endif
