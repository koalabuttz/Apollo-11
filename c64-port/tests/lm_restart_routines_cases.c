/* SPDX-License-Identifier: MIT */
#include "lm_restart_routines_cases.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
static enum exec_status service(const agc_native_program *p,apollo_overlay_state *s,void *io){lm_rsr_log *log=io;lm_rsr_call *c;agc_word q=s->words[2];
 if(log->calls>=LM_RSR_MAX_CALLS)return EXEC_OUT_OF_SCOPE;
 c=&log->call[log->calls++];c->service=s->words[5];c->a=s->words[0];c->l=s->words[1];c->q=q;
 if(!agc_transfer_operand(p,s,0,q,&c->cadr)||!agc_transfer_operand(p,s,0,(agc_word)(q+1u),&c->bbcon))return EXEC_OUT_OF_SCOPE;
 s->words[5]=(agc_word)(q+2u);return EXEC_OK;
}
static const agc_routine_entry services[4]={{FF(LM_RS_NOVAC),service},{FF(LM_RS_FINDVAC),service},{FF(LM_RS_WAITLIST),service},{FF(LM_RS_LONGCALL),service}};
enum exec_status lm_rsr_run(apollo_overlay_state *s,uint16_t n,lm_rsr_log *log,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,lm_restart_operands,2,AGC_TRANSFER_NONE,0,0,0,0,0};agc_word superbank=0;
 tables[0]=lm_restart_routines;tables[1].entries=services;tables[1].count=4;program.basic=tables;program.basic_count=2;
 (void)lm_rsselect_input(s,lm_rsr_select_case(n));log->calls=0;
 return agc_transfer_run(&program,s,log,&superbank,FF(LM_RS_SWRETURN),32,steps);
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
static void mix_image(const apollo_overlay_state *s){unsigned i;for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));}
uint32_t lm_rs_select_digest(apollo_overlay_state *s,uint16_t n){enum lm_restart_select_exit e=lm_restart_select(s,lm_rsselect_input(s,n));
 sum_a=sum_b=0;mix((uint16_t)(e==LM_RS_SELECTED_TASK?1u:e==LM_RS_SELECTED_LONGCALL?2u:e==LM_RS_SELECTED_DISPLAY?3u:e==LM_RS_SELECT_INVALID?4u:0u));mix_image(s);
 return (uint32_t)sum_b<<16|sum_a;
}
uint32_t lm_rs_compose_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){lm_rsr_log log;uint16_t steps=0;unsigned i;
 *status=lm_rsr_run(s,n,&log,&steps);sum_a=sum_b=0;mix(log.calls);
 for(i=0;i<log.calls;++i){mix(log.call[i].service);mix(log.call[i].a);mix(log.call[i].l);mix(log.call[i].q);mix(log.call[i].cadr);mix(log.call[i].bbcon);}
 mix_image(s);return (uint32_t)sum_b<<16|sum_a;
}
