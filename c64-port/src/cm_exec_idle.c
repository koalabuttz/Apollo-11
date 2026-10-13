/* SPDX-License-Identifier: MIT */
#include "cm_exec_routines.h"
#include "cm_idle_dispatch.h"
#include "cm_idle_recheck.h"
#include "cm_super_dispatch.h"
#include "cm_svct3.h"
#include "agc_image_w.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))

/* The idle loop, SUPDXCHZ and the SVCT3 end task. Each composes the
 * qualified image body of its source path and leaves Z where the original
 * continues. A completed idle dispatch is EXEC_OK (a job) or EXEC_IDLE (the
 * self-check at SELFRET); both are continuations. */
static enum exec_status landed(enum exec_status st){return st==EXEC_OK||st==EXEC_IDLE?EXEC_OK:EXEC_OUT_OF_SCOPE;}
/* ADVAN 01,3231: CCS NEWJOB, then NUCHANG2, NUDIRECT or the self-check. */
static enum exec_status advan(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?landed(cm_idle_dispatch(s,v,&t)):EXEC_OUT_OF_SCOPE;
}
/* NUCHANG2 01,3032 rejoins ADVAN's tails; Z and the interrupt bit it sets
 * on the way are put back if the tail refuses. */
static enum exec_status nuchang2(const agc_native_program *p,apollo_overlay_state *s,void *v){cm_native_io *io=v;exec_transfer t;agc_word z,irq;enum exec_status st;(void)p;
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 z=W[5];irq=io->interrupt_enabled;st=landed(cm_idle_recheck(s,io,&t));
 if(st!=EXEC_OK){W[5]=z;io->interrupt_enabled=irq;}
 return st;
}
/* DUMMYJOB 01,3223: CS ZERO; TS NEWJOB; then (DUMMYJOB+2, where the restart
 * enters) RELINT; CS TWO; EXTEND; WAND DSALMOUT; ADVAN. A is left -2 AND
 * channel 11, but every ADVAN tail loads A again. */
static enum exec_status dummyjob(const agc_native_program *p,apollo_overlay_state *s,void *v){cm_native_io *io=v,io0;exec_transfer t;agc_word z,newjob;enum exec_status st;(void)p;
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 z=W[5];newjob=W[067];io0=*io;
 if(z==CM_EXEC_DUMMYJOB)W[067]=AGC_NEG_ZERO;
 io->interrupt_enabled=1;io->channel11=(agc_word)(io->channel11&~2u);W[5]=CM_EXEC_ADVAN;
 st=landed(cm_idle_dispatch(s,io,&t));
 if(st!=EXEC_OK){W[5]=z;W[067]=newjob;*io=io0;}
 return st;
}
/* SUPDXCHZ 05227 (2CADR in A,L) and SUPDXCHZ+1 05230 (in L,A). */
static enum exec_status supdxchz(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?cm_super_dispatch(s,v,&t):EXEC_OUT_OF_SCOPE;
}
/* SVCT3 05306, its NOVAC return 05323 and SVCT3X's TC SVCT3 at 07721. */
static enum exec_status svct3(const agc_native_program *p,apollo_overlay_state *s,void *v){uint8_t phase;(void)p;
 if(!s||!v)return EXEC_OUT_OF_SCOPE;
 phase=W[5]==CM_EXEC_SVCT3?0u:W[5]==CM_EXEC_SVCT3_NOVAC_RETURN?1u:2u;
 return cm_svct3_phase(s,v,phase)!=CM_SVCT3_INVALID?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static const agc_routine_entry entries[9]={
 {B1(CM_EXEC_NUCHANG2),nuchang2},{B1(CM_EXEC_DUMMYJOB),dummyjob},{B1(CM_EXEC_DUMMYJOB+2u),dummyjob},{B1(CM_EXEC_ADVAN),advan},
 {FF(CM_EXEC_SUPDXCHZ),supdxchz},{FF(CM_EXEC_SUPDXCHZ+1u),supdxchz},
 {FF(CM_EXEC_SVCT3),svct3},{FF(CM_EXEC_SVCT3_NOVAC_RETURN),svct3},{FF(CM_EXEC_SVCT3X_RETURN),svct3}};
const agc_routine_table cm_exec_idle_routines={entries,9};
