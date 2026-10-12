/* SPDX-License-Identifier: MIT */
#include "lm_exec_idle_cases.h"
#include "lm_idle_dispatch_cases.h"
#include "lm_super_dispatch_cases.h"
#include "lm_svct3_cases.h"
static const uint16_t start[LM_EXI_COHORTS+1]={0,1024,1792,2304,3328,4864};
enum lm_exi_cohort lm_exi_cohort(uint16_t n){unsigned k=0;while(k+1u<LM_EXI_COHORTS&&n>=start[k+1u])++k;return (enum lm_exi_cohort)k;}
uint16_t lm_exi_first(enum lm_exi_cohort k){return start[k];}
void lm_exi_input(apollo_overlay_state *m,p66_bias_io *io,agc_hardware *hw,uint16_t n){enum lm_exi_cohort k=lm_exi_cohort(n);uint32_t j=(uint32_t)(n-start[k]),i;
 hw->channel11=0;hw->in_interrupt=0;
 switch(k){
 /* The generator's ADVAN states (0-8191: NEWJOB +0, a basic job, an
  * interpretive job, -0) and NUCHANG2 states (8192-14335). */
 case LM_EXI_ADVAN:lm_idle_dispatch_input(m,io,&hw->channel11,(uint16_t)(j*8u+j/128u));break;
 case LM_EXI_NUCHANG2:lm_idle_dispatch_input(m,io,&hw->channel11,(uint16_t)(8192u+j*8u+j/96u));break;
 /* DUMMYJOB (even) and DUMMYJOB+2 (odd) over the ADVAN states, with SELFRET
  * anywhere in the self-check bank and channel 11 varied. */
 case LM_EXI_DUMMYJOB:lm_idle_dispatch_input(m,io,&hw->channel11,(uint16_t)(j*16u+j/32u));
  m->words[5]=(agc_word)(j&1u?LM_EXEC_DUMMYJOB+2u:LM_EXEC_DUMMYJOB);m->words[LM_EXEC_SELFRET]=(agc_word)(02000u+(j*0151u)%02000u);
  hw->channel11=(agc_word)((j*0123u)&AGC_MASK);break;
 /* SUPDXCHZ and SUPDXCHZ+1. SUPDXCHZ's own first step lands on +1, so a
  * SUPDXCHZ case targeting +1 would be indistinguishable there: skipped. */
 case LM_EXI_SUPDXCHZ:i=(j*16u+j/64u)%LM_SUPER_DISPATCH_CASES;
  for(;;){lm_super_dispatch_input(m,io,(uint16_t)i);if(!(m->words[5]==LM_EXEC_SUPDXCHZ&&agc_correct(m->words[0])==LM_EXEC_SUPDXCHZ+1u))break;i=(i+64u)%LM_SUPER_DISPATCH_CASES;}
  break;
 /* SVCT3 runs as a Waitlist task, inside T3RUPT. */
 default:lm_svct3_input(m,io,(uint16_t)j);hw->in_interrupt=1;break;
 }
}
unsigned lm_exi_run(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&lm_exec_idle_routines;program.basic_count=1;program.hardware=hw;
 lm_exi_input(s,io,hw,n);
 call=agc_transfer_resolve(&program,s,io->superbank);
 if(!call)return 3u;
 if(call(&program,s,io)!=EXEC_OK)return LM_EXI_REFUSED;
 return s->words[5]==LM_EXEC_BAILOUT1||s->words[5]==LM_EXEC_POODOO1?LM_EXI_ALARM:LM_EXI_RETURNED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_exi_digest(apollo_overlay_state *s,uint16_t n){p66_bias_io io;agc_hardware hw;unsigned i,kind=lm_exi_run(s,&io,&hw,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LM_EXI_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.superbank);mix(io.interrupt_enabled);mix(hw.channel11);mix(hw.in_interrupt);}
 return (uint32_t)sum_b<<16|sum_a;
}
