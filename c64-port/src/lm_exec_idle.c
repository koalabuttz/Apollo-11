/* SPDX-License-Identifier: MIT */
#include "lm_exec_routines.h"
#include "lm_idle_dispatch.h"
#include "lm_super_dispatch.h"
#include "lm_svct3.h"
#include "agc_image_w.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))

/* The idle loop, SUPDXCHZ and the SVCT3 end task. Each composes the
 * qualified image body of its source path and leaves Z where the original
 * continues. A completed idle dispatch is EXEC_OK (a job) or EXEC_IDLE (the
 * self-check at SELFRET); both are continuations. COMP ACTY (channel 11 bit
 * 2) is in program->hardware. */
static enum exec_status landed(enum exec_status st){return st==EXEC_OK||st==EXEC_IDLE?EXEC_OK:EXEC_OUT_OF_SCOPE;}
static enum exec_status idle(const agc_native_program *p,apollo_overlay_state *s,void *v,enum lm_idle_dispatch_entry e){exec_transfer t;
 return p&&p->hardware&&s&&v?landed(lm_idle_dispatch(s,v,&p->hardware->channel11,e,&t)):EXEC_OUT_OF_SCOPE;
}
/* ADVAN 01,3214: CCS NEWJOB, then NUCHANG2, NUDIRECT or the self-check. */
static enum exec_status advan(const agc_native_program *p,apollo_overlay_state *s,void *v){return idle(p,s,v,LM_IDLE_ADVAN);}
/* NUCHANG2 01,3011 (INHINT, CCS NEWJOB, CHANJOB or ADVAN+2). */
static enum exec_status nuchang2(const agc_native_program *p,apollo_overlay_state *s,void *v){return idle(p,s,v,LM_IDLE_NUCHANG2);}
/* DUMMYJOB 01,3206: CS ZERO; TS NEWJOB; then (DUMMYJOB+2, where the restart
 * enters) RELINT; CS TWO; EXTEND; WAND DSALMOUT; ADVAN. A is left -2 AND
 * channel 11, but every ADVAN tail loads A again. */
static enum exec_status dummyjob(const agc_native_program *p,apollo_overlay_state *s,void *v){p66_bias_io *io=v,io0;agc_word z,newjob,ch11;enum exec_status st;
 if(!p||!p->hardware||!s||!io)return EXEC_OUT_OF_SCOPE;
 z=W[5];newjob=W[067];io0=*io;ch11=p->hardware->channel11;
 if(z==LM_EXEC_DUMMYJOB)W[067]=AGC_NEG_ZERO;
 io->interrupt_enabled=1;p->hardware->channel11=(agc_word)(ch11&~2u);W[5]=LM_EXEC_ADVAN;
 st=idle(p,s,v,LM_IDLE_ADVAN);
 if(st!=EXEC_OK){W[5]=z;W[067]=newjob;*io=io0;p->hardware->channel11=ch11;}
 return st;
}
/* SUPDXCHZ 05165 (2CADR in A,L) and SUPDXCHZ+1 05166 (in L,A). */
static enum exec_status supdxchz(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?lm_super_dispatch(s,v,&t):EXEC_OUT_OF_SCOPE;
}
/* SVCT3 05240, its NOVAC return 05255 and SVCT3X's TC SVCT3 at 05260. */
static enum exec_status svct3(const agc_native_program *p,apollo_overlay_state *s,void *v){uint8_t phase;(void)p;
 if(!s||!v)return EXEC_OUT_OF_SCOPE;
 phase=W[5]==LM_EXEC_SVCT3?0u:W[5]==LM_EXEC_SVCT3_NOVAC_RETURN?1u:2u;
 return lm_svct3_phase(s,v,phase)!=LM_SVCT3_INVALID?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static const agc_routine_entry entries[9]={
 {B1(LM_EXEC_NUCHANG2),nuchang2},{B1(LM_EXEC_DUMMYJOB),dummyjob},{B1(LM_EXEC_DUMMYJOB+2u),dummyjob},{B1(LM_EXEC_ADVAN),advan},
 {FF(LM_EXEC_SUPDXCHZ),supdxchz},{FF(LM_EXEC_SUPDXCHZ+1u),supdxchz},
 {FF(LM_EXEC_SVCT3),svct3},{FF(LM_EXEC_SVCT3_NOVAC_RETURN),svct3},{FF(LM_EXEC_SVCT3X_RETURN),svct3}};
const agc_routine_table lm_exec_idle_routines={entries,9};
