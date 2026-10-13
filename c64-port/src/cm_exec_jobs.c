/* SPDX-License-Identifier: MIT */
#include "cm_exec_routines.h"
#include "cm_admission_zero.h"
#include "cm_context_change.h"
#include "cm_basic_sleep.h"
#include "cm_wake_complete.h"
#include "cm_priority_complete.h"
#include "cm_basic_finish.h"
#define W s->words
#define FF(z) (uint16_t)(2048u+((z)-04000u))

/* Executive job services. Each composes the qualified image body of its
 * source path, which leaves Z at the source continuation: the caller, the
 * selected job (its basic entry, or NEWOPS 06037 after INTRSM for an
 * interpretive one), ADVAN 01,3231 when nothing is ready, or JOBWAKE's
 * exceptional transfers (TC 014 for a +0 priority word, CCSHOLE 05747). The
 * Comanche bodies keep channel 11 in cm_native_io. */
static enum exec_status spvac(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_address none={0,0};enum exec_status st;(void)p;
 if(!s||!v)return EXEC_OUT_OF_SCOPE;
 st=cm_admission_zero(s,v,W[5]==CM_EXEC_SPVAC?CM_ADMIT_SPVAC:CM_ADMIT_SPVACIN,none);
 return st==EXEC_OK||st==EXEC_NO_VAC||st==EXEC_NO_CORE?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status change(apollo_overlay_state *s,void *v,uint8_t interpretive){exec_transfer t;
 return s&&v?cm_context_change(s,v,interpretive,&t):EXEC_OUT_OF_SCOPE;
}
static enum exec_status chang1(const agc_native_program *p,apollo_overlay_state *s,void *v){(void)p;return change(s,v,0);}
static enum exec_status chang2(const agc_native_program *p,apollo_overlay_state *s,void *v){(void)p;return change(s,v,1);}
/* A completed job switch (EXEC_IDLE: none ready, at ADVAN) or JOBWAKE
 * finding no sleeping job (EXEC_NOT_FOUND, back at the caller). */
static enum exec_status done(enum exec_status st){return st==EXEC_OK||st==EXEC_IDLE||st==EXEC_NOT_FOUND?EXEC_OK:EXEC_OUT_OF_SCOPE;}
static enum exec_status jobsleep(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?done(cm_basic_sleep(s,v,&t)):EXEC_OUT_OF_SCOPE;
}
static enum exec_status jobwake(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?done(cm_wake_complete(s,v,&t)):EXEC_OUT_OF_SCOPE;
}
static enum exec_status priochng(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?done(cm_priority_complete(s,v,&t)):EXEC_OUT_OF_SCOPE;
}
static enum exec_status endofjob(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?done(cm_basic_finish(s,v,&t)):EXEC_OUT_OF_SCOPE;
}
static const agc_routine_entry entries[8]={
 {FF(CM_EXEC_SPVACIN),spvac},{FF(CM_EXEC_SPVAC),spvac},{FF(CM_EXEC_CHANG1),chang1},{FF(CM_EXEC_CHANG2),chang2},
 {FF(CM_EXEC_JOBSLEEP),jobsleep},{FF(CM_EXEC_JOBWAKE),jobwake},{FF(CM_EXEC_PRIOCHNG),priochng},{FF(CM_EXEC_ENDOFJOB),endofjob}};
const agc_routine_table cm_exec_job_routines={entries,8};
