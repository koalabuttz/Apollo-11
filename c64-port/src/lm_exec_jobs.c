/* SPDX-License-Identifier: MIT */
#include "lm_exec_routines.h"
#include "lm_admission_image.h"
#include "lm_basic_sleep.h"
#include "lm_basic_priority.h"
#include "lm_basic_finish.h"
#include "lm_basic_change.h"
#include "lm_interpreter_change.h"
#include "agc_image_w.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))

/* Executive job services. Each composes the qualified image body of its
 * source path; the job switches end at the selected job (its basic entry, or
 * NEWOPS 06070 after INTRSM for an interpretive one), at ADVAN 01,3214 when
 * nothing is ready, or back at the caller. */
static enum exec_status spvac(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_address none={0,0};enum exec_status st;(void)p;
 if(!s||!v)return EXEC_OUT_OF_SCOPE;
 st=lm_admission_image(s,v,W[5]==LM_EXEC_SPVAC?LM_ADMIT_SPVAC:LM_ADMIT_SPVACIN,none);
 return st==EXEC_OK||st==EXEC_NO_VAC||st==EXEC_NO_CORE?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status chang1(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?lm_basic_change(s,v,LM_CHANG1_ENTRY,&t):EXEC_OUT_OF_SCOPE;
}
static enum exec_status chang2(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;(void)p;
 return s&&v?lm_interpreter_change(s,v,&t):EXEC_OUT_OF_SCOPE;
}
/* EJSCAN's choice (JOBSLEEP from no candidate, PRIOCHNG from the new
 * priority): the first core set of highest priority-1 above floor. */
static uint8_t candidate(const apollo_overlay_state *s,uint16_t floor){uint8_t i,win=0;
 for(i=1;i<8u;++i){uint16_t pr=W[0167u+12u*i];if(pr&&pr<040000u&&(uint16_t)(pr-1u)>floor){floor=(uint16_t)(pr-1u);win=i;}}
 return win;
}
/* The basic exchange bodies select only basic jobs. An interpretive successor
 * (negative LOC) leaves CHANJOB the same way with its LOC already turned into
 * the INTRSM resume address (ENDPRCHG: COM, AD ONE, TS LOC); INTRSM then
 * rebuilds BANKSET, INTBIT15 and EDOP and goes to NEWOPS, as in ENDOFJOB. */
static int interpretive_in(apollo_overlay_state *s,uint8_t win,agc_word *saved){agc_word loc;
 if(!win)return 0;
 loc=W[0164u+12u*win];*saved=loc;
 if(!(loc&AGC_SIGN))return 0;
 if(agc_negate(loc)>=07777u)return -1;
 W[0164u+12u*win]=agc_add_word(agc_negate(loc),1);return 1;
}
static void intrsm(apollo_overlay_state *s,agc_word loc){
 W[0164]=loc;W[0165]=W[6];W[0115]=(agc_word)(W[6]&040000u);W[023]=0;W[0]=agc_extend(W[0115]);W[5]=06070;
}
static enum exec_status jobsleep(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_address next;agc_word saved=0;uint8_t win;int in;enum exec_status st;
 if(!p||!p->hardware||!s||!v||W[5]!=LM_EXEC_JOBSLEEP)return EXEC_OUT_OF_SCOPE;
 win=candidate(s,0);in=interpretive_in(s,win,&saved);if(in<0)return EXEC_OUT_OF_SCOPE;
 st=lm_basic_sleep(s,v,&p->hardware->channel11,&next);
 if(st!=EXEC_OK&&st!=EXEC_IDLE){if(in)W[0164u+12u*win]=saved;return EXEC_OUT_OF_SCOPE;}
 if(in)intrsm(s,next.location);
 return EXEC_OK;
}
static enum exec_status jobwake(const agc_native_program *p,apollo_overlay_state *s,void *v){enum exec_status st;(void)p;
 if(!s||!v)return EXEC_OUT_OF_SCOPE;
 st=lm_basic_wake(s,v);
 return st==EXEC_OK||st==EXEC_NOT_FOUND?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status priochng(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_address next;agc_word saved=0;uint8_t win;int in;enum exec_status st;(void)p;
 if(!s||!v||W[5]!=LM_EXEC_PRIOCHNG)return EXEC_OUT_OF_SCOPE;
 win=candidate(s,(uint16_t)(W[0]+(W[0167]&0777u)));in=interpretive_in(s,win,&saved);if(in<0)return EXEC_OUT_OF_SCOPE;
 st=lm_basic_priority_change(s,v,&next);
 if(st!=EXEC_OK){if(in)W[0164u+12u*win]=saved;return EXEC_OUT_OF_SCOPE;}
 if(in)intrsm(s,next.location);
 return EXEC_OK;
}
static enum exec_status endofjob(const agc_native_program *p,apollo_overlay_state *s,void *v){exec_transfer t;enum exec_status st;
 if(!p||!p->hardware||!s||!v)return EXEC_OUT_OF_SCOPE;
 st=lm_basic_finish(s,v,&p->hardware->channel11,&t);
 return st==EXEC_OK||st==EXEC_IDLE?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static const agc_routine_entry entries[8]={
 {FF(LM_EXEC_SPVACIN),spvac},{FF(LM_EXEC_SPVAC),spvac},{FF(LM_EXEC_CHANG1),chang1},{FF(LM_EXEC_CHANG2),chang2},
 {FF(LM_EXEC_JOBSLEEP),jobsleep},{FF(LM_EXEC_JOBWAKE),jobwake},{FF(LM_EXEC_PRIOCHNG),priochng},{FF(LM_EXEC_ENDOFJOB),endofjob}};
const agc_routine_table lm_exec_job_routines={entries,8};
