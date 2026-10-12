/* SPDX-License-Identifier: MIT */
#ifndef LM_RESTART_SELECT_H
#define LM_RESTART_SELECT_H
#include "overlay_runtime.h"
/* Luminary099 RESTARTS (01,3526), GETPART2 (01,3541) and PHSPART2 (01,3763)
 * through the transfer to the service the restart selects: TC GOLOC-1 with
 * GOLOC-1 holding FINDVAC/NOVAC/WAITLIST/LONGCALL and GOLOC the 2CADR, or the
 * X.1 display restart's TC FINDVAC (2CADR INITDSP). Group index is MPAC+5,
 * phase MPAC; FB 1/EB 3 (BB 02003) as GOPROG sets them. The table words are
 * compiled source data (src/lm_restart_table_data.inc); the one fixed duration
 * a table entry reads, 600SECS (13,2630), is the source constant (3,025140).
 * No suspended C stack: the caller runs the selected service, then the TC its
 * return reaches at GOLOC+2. The same instruction sequence as Comanche055
 * RESTARTS (cm_restart_select), at Luminary addresses. */
enum lm_restart_select_entry { LM_RS_SELECT_GROUP=0,LM_RS_SELECT_PART2=1,LM_RS_SELECT_TYPEB_TABLE=2 };
enum lm_restart_select_exit { LM_RS_SELECTED_JOB=0,LM_RS_SELECTED_TASK=1,LM_RS_SELECTED_LONGCALL=2,LM_RS_SELECTED_DISPLAY=3,LM_RS_SELECT_INVALID=4 };
enum lm_restart_select_exit lm_restart_select(apollo_overlay_state *,enum lm_restart_select_entry);

/* Original addresses the selection hands control to. */
#define LM_RS_RESTARTS 03526u
#define LM_RS_GETPART2 03541u
#define LM_RS_PHSPART2 03763u
#define LM_RS_DISPLAY_RETURN 03550u /* TC RTRNCADR after 2CADR INITDSP */
#define LM_RS_RTRNCADR 03565u       /* TC SWRETURN */
#define LM_RS_FINDVAC 05105u
#define LM_RS_NOVAC 05072u
#define LM_RS_WAITLIST 05203u
#define LM_RS_LONGCALL 05277u
#define LM_RS_SWRETURN 04631u
#define LM_RS_GOLOC 0705u
#endif
