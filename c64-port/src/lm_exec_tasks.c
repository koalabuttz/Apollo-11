/* SPDX-License-Identifier: MIT */
#include "lm_exec_routines.h"
#include "lm_delay_image.h"
#include "lm_taskover_resume.h"
#include "lm_waitlist_dispatch.h"
#include "lm_waitlist_irq_dispatch.h"
#define W s->words
#define FF(z) (uint16_t)(2048u+((z)-04000u))

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
 if(W[0734]==AGC_NEG_ZERO)return lm_taskover_resume(s,v,&p->hardware->in_interrupt);
 return dispatch(p->hardware,s,v);
}
static const agc_routine_entry entries[4]={
 {FF(LM_EXEC_T3RUPT_VECTOR),t3rupt},{FF(LM_EXEC_FIXDELAY),fixdelay},{FF(LM_EXEC_VARDELAY),vardelay},{FF(LM_EXEC_TASKOVER),taskover}};
const agc_routine_table lm_exec_task_routines={entries,4};
