/* SPDX-License-Identifier: MIT */
#include "lm_exec_routines.h"
#include "lm_delay_image.h"
#include "lm_taskover_resume.h"
#include "lm_waitlist_dispatch.h"
#include "lm_waitlist_irq_dispatch.h"
#define W s->words
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))

/* The inline delay word at Q. */
static int operand(const agc_native_program *p,const apollo_overlay_state *s,const p66_bias_io *io,agc_word *d){
 return p&&io&&W[2]<=07776u&&agc_transfer_operand(p,s,io->superbank,W[2],d);
}
/* FIXDELAY (the delay is the DEC at Q) and VARDELAY (delay in A): the calling
 * task's remainder goes on the Waitlist and control reaches TASKOVER, or
 * BAILOUT1/POODOO1 with 1203/1204. */
static enum exec_status delay(const agc_native_program *p,apollo_overlay_state *s,void *v,enum lm_delay_entry entry){agc_word d=0;enum wl_status st;
 if(!s||!v)return EXEC_OUT_OF_SCOPE;
 if(entry==LM_FIXDELAY_ENTRY&&W[2]>=02000u&&!operand(p,s,v,&d))return EXEC_OUT_OF_SCOPE;
 st=lm_delay_image(s,v,entry,d);
 return st==WL_OK||st==WL_FULL||st==WL_BAD_DELAY?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status fixdelay(const agc_native_program *p,apollo_overlay_state *s,void *v){return delay(p,s,v,LM_FIXDELAY_ENTRY);}
static enum exec_status vardelay(const agc_native_program *p,apollo_overlay_state *s,void *v){return delay(p,s,v,LM_VARDELAY_ENTRY);}
/* T3RUPT2: shift the lists and DTCB to the next task (from the T3 vector
 * 04014 lead-in, or TASKOVER with RUPTAGN +1). The interrupt-enable bit is
 * whatever the interrupted code or the finished task left. */
static enum exec_status dispatch(agc_hardware *hw,apollo_overlay_state *s,p66_bias_io *io){exec_address next;enum wl_status st;
 st=io->interrupt_enabled?lm_waitlist_irq_dispatch(s,io,hw->in_interrupt,&next):lm_waitlist_dispatch(s,io,hw->in_interrupt,&next);
 return st==WL_OK?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status t3rupt(const agc_native_program *p,apollo_overlay_state *s,void *v){
 if(!p||!p->hardware||!s||!v)return EXEC_OUT_OF_SCOPE;
 return dispatch(p->hardware,s,v);
}
/* TASKOVER: RUPTAGN -0 resumes the interrupted program (Z = ZRUPT-1, the
 * instruction saved in BRUPT, ends the interrupt); +1 dispatches the next
 * task that fell due. */
static enum exec_status taskover(const agc_native_program *p,apollo_overlay_state *s,void *v){
 if(!p||!p->hardware||!s||!v)return EXEC_OUT_OF_SCOPE;
 /* RESUME: BRUPT must be the word at ZRUPT-1 under BANKRUPT's banks (#144). */
 if(W[0734]==AGC_NEG_ZERO){if(!agc_resume_exact(p,s,(agc_word)(W[016]&076000u),(agc_word)((W[016]&7u)<<8),((p66_bias_io *)v)->superbank))return EXEC_OUT_OF_SCOPE;
  return lm_taskover_resume(s,v,&p->hardware->in_interrupt);}
 return dispatch(p->hardware,s,v);
}
/* T3RUPT 01,3407, where the T3 vector's TCF T3RUPT lands (BB = T3RPTBB, A the
 * interrupted BB): EXTEND; ROR SUPERBNK; TS BANKRUPT; EXTEND; QXCH QRUPT, then
 * T3RUPT2 as TASKOVER reaches it (it starts with CAF NEG1/2 and sets RUPTAGN
 * itself). A refused T3RUPT2 leaves the image as it was. */
static const uint16_t t3words[6]={0,2,5,012,016,0734};
static enum exec_status t3rupt_body(const agc_native_program *p,apollo_overlay_state *s,void *v){p66_bias_io *io=v;agc_word saved[6],a,q;uint8_t i;enum exec_status st;
 if(!p||!p->hardware||!s||!io||p->hardware->in_interrupt!=1u||W[7]||W[6]!=02003u||agc_overflow(W[0]))return EXEC_OUT_OF_SCOPE;
 a=agc_correct(W[0]);if(a&(agc_word)~076007u)return EXEC_OUT_OF_SCOPE;
 for(i=0;i<6u;++i)saved[i]=W[t3words[i]];
 W[0]=(agc_word)(a|io->superbank);W[016]=W[0];q=W[2];W[2]=agc_extend(W[012]);W[012]=agc_correct(q);
 W[5]=LM_EXEC_TASKOVER;W[0734]=1;st=dispatch(p->hardware,s,io);
 if(st!=EXEC_OK)for(i=0;i<6u;++i)W[t3words[i]]=saved[i];
 return st;
}
/* DLY2 -1 05211, from DELAYJOB's TCF DLY2 -1 (A the WAKER CADR, L the task
 * BBCON, Q the delay, WAITEXIT = TCSLEEP): TS WAITADR; CAF WAITBB; XCH BBANK;
 * TCF WAIT2, then the WAIT2 insertion back to WAITEXIT +2, or BAILOUT1. */
static const uint16_t dlywords[8]={0,3,4,5,6,063,026,0734};
static enum exec_status dly2_m1(const agc_native_program *p,apollo_overlay_state *s,void *v){p66_bias_io *io=v;agc_word saved[8],list[26];uint8_t i;enum wl_status st;
 (void)p;if(!s||!io||W[7]||io->interrupt_enabled||agc_overflow(W[0]))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<8u;++i)saved[i]=W[dlywords[i]];
 for(i=0;i<26u;++i)list[i]=W[01400u+i];
 W[063]=agc_correct(W[0]);W[0]=agc_extend(W[6]);W[3]=01400;W[4]=02000;W[6]=02003;W[5]=03231;
 st=lm_waitlist_internal(s,io);
 if(st==WL_OK||st==WL_FULL||st==WL_BAD_DELAY)return EXEC_OK;
 for(i=0;i<8u;++i)W[dlywords[i]]=saved[i];
 for(i=0;i<26u;++i)W[01400u+i]=list[i];
 return EXEC_OUT_OF_SCOPE;
}
static const agc_routine_entry entries[6]={
 {B1(03407u),t3rupt_body},
 {FF(LM_EXEC_T3RUPT_VECTOR),t3rupt},{FF(LM_EXEC_DLY2_M1),dly2_m1},{FF(LM_EXEC_FIXDELAY),fixdelay},{FF(LM_EXEC_VARDELAY),vardelay},{FF(LM_EXEC_TASKOVER),taskover}};
const agc_routine_table lm_exec_task_routines={entries,6};
