/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_SELECT_H
#define CM_RESTART_SELECT_H
#include "cm_restart_timing.h"
#include "executive.h"
enum cm_restart_select_entry { CM_RS_SELECT_GROUP=0,CM_RS_SELECT_PART2=1,CM_RS_SELECT_TYPEB_TABLE=2 };
enum cm_restart_select_exit { CM_RS_SELECTED_JOB=0,CM_RS_SELECTED_TASK=1,CM_RS_SELECTED_LONGCALL=2,CM_RS_SELECTED_DISPLAY=3,CM_RS_SELECT_INVALID=4 };
/* Original RESTARTS, PHSPART2 and GETPART2 through the actual service transfer.
 * Source group index is MPAC+5, phase is MPAC; fixed/erasable bank1/3 required.
 * Table priorities/times/2CADRs are source data, not executable rope. The only
 * fixed duration used by table2.3 is original 600SECS (60000 centiseconds).
 * The caller executes source-selected Executive/WAITLIST/LONGCALL native code
 * before entering the next table part; no suspended C stack is retained. */
enum cm_restart_select_exit cm_restart_select(apollo_overlay_state *,enum cm_restart_select_entry);
#endif
