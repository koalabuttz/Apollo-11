/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_ROUTINES_H
#define CM_RESTART_ROUTINES_H
#include "agc_transfer.h"
/* Comanche055 RESTARTS_ROUTINE and RESTART_TABLES as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md).
 *
 * RESTARTS (01,3520), GETPART2 (01,3533) and PHSPART2 (01,3755) select a
 * restart from the compiled RESTART_TABLES words (src/cm_restart_table_data.inc)
 * and leave Z at TC GOLOC-1, with GOLOC-1 holding FINDVAC/NOVAC/WAITLIST/
 * LONGCALL and GOLOC the 2CADR, or at the X.1 display restart's TC FINDVAC
 * (2CADR INITDSP in cm_restart_operands). Group index is MPAC+5, phase MPAC;
 * FB 1/EB 3 (BB 02003) as GOPROG sets them. The one fixed duration a table
 * entry reads, 600SECS (13,2654), is the source constant (3,025140).
 * The instruction sequence is Luminary099's (lm_restart_select) at Comanche
 * addresses; the source files differ only in COUNT cards.
 *
 * Also registered: the display return 01,3542 (TC RTRNCADR, TC SWRETURN), the
 * two erasable words RESTARTS stores for later execution, GOLOC-1 and GOLOC+2,
 * at their erasable keys, and NEWPHASE (4114), whose group operand comes from
 * cm_restart_operands (the nine `TC NEWPHASE` sites). Any other word at
 * GOLOC-1/GOLOC+2 is EXEC_OUT_OF_SCOPE: no original producer stores it. */
enum cm_rr_entry { CM_RR_GROUP=0,CM_RR_PART2=1,CM_RR_TYPEB_TABLE=2 };
enum cm_rr_exit { CM_RR_SEL_JOB=0,CM_RR_SEL_TASK=1,CM_RR_SEL_LONGCALL=2,CM_RR_SEL_DISPLAY=3,CM_RR_INVALID=4 };
enum cm_rr_exit cm_restart_routines_select(apollo_overlay_state *,enum cm_rr_entry);

#define CM_RR_RESTARTS 03520u
#define CM_RR_GETPART2 03533u
#define CM_RR_PHSPART2 03755u
#define CM_RR_DISPLAY_RETURN 03542u /* TC RTRNCADR after 2CADR INITDSP */
#define CM_RR_RTRNCADR 03557u       /* TC SWRETURN */
#define CM_RR_FINDVAC 05147u
#define CM_RR_NOVAC 05134u
#define CM_RR_WAITLIST 05245u
#define CM_RR_LONGCALL 05342u
#define CM_RR_SWRETURN 04675u
#define CM_RR_NEWPHASE 04114u
#define CM_RR_GOLOC 0705u

extern const agc_routine_table cm_restart_routines;
/* 2CADR INITDSP (01,3540-3541), then the NEWPHASE group operands, sorted. */
#define CM_RESTART_OPERAND_COUNT 11u /* 2 + CM_NEWPHASE_SITE_COUNT */
extern const agc_site_operand cm_restart_operands[CM_RESTART_OPERAND_COUNT];
#endif
