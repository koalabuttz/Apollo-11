/* SPDX-License-Identifier: MIT */
#ifndef CM_EXECUTIVE_H
#define CM_EXECUTIVE_H
#include "executive.h"
#define CM_EXEC_CORE_COUNT 7
#define CM_EXEC_ADMISSION_COUNT 7
#define CM_EXEC_CORE_WORDS 12
#define CM_EXEC_VAC_COUNT 5
#define CM_EXEC_VAC_WORDS 44
#define CM_EXEC_LOW9 UINT16_C(0777)
#define CM_EXEC_FAKEPRET UINT16_C(0110)
#define CM_EXEC_VAC1USE UINT16_C(0400)
enum cm_exec_core_field { CM_EXEC_MPAC=0, CM_EXEC_MODE=7, CM_EXEC_LOC=8,
 CM_EXEC_BANK=9, CM_EXEC_PUSH=10, CM_EXEC_PRIORITY=11 };
typedef struct {
 agc_word core[CM_EXEC_CORE_COUNT][CM_EXEC_CORE_WORDS];
 agc_word vac[CM_EXEC_VAC_COUNT][CM_EXEC_VAC_WORDS];
 agc_word newjob,newprio,newloc[2],locctr,fixloc,ovfind,alarm;
} cm_executive;
/* Comanche 055 has SEVEN core sets. Initialization is only local state. */
void cm_exec_init(cm_executive *state);
/* Stops at ENDFIND or BAILOUT entry (no CM alarm/restart effects).
 * Priority is 01000..037000; raw location/bank are 15-bit words.
 * NEWJOB is -0 or a multiple of 12 through 72. Existing jobs and VAC
 * bases must be valid admitted representations, and idle slot-zero
 * admission requires NEWJOB=-0 (CCSHOLE states are outside scope).
 * The caller owns interrupt exclusion and distinct owned objects. */
enum exec_status cm_exec_admit(cm_executive *state,agc_word priority,
 agc_word location,agc_word bank,int needs_vac);
/* Releases core zero/VAC and selects slot 1..6 or -1 idle; stops before
 * CHANJOB-2 / DUMMYJOB dispatch. No instruction decoder/native task call. */
int cm_exec_endjob_select(cm_executive *state);
#endif
