/* SPDX-License-Identifier: MIT */
#include "cm_exec_idle_cases.h"
#include "cm_idle_dispatch_cases.h"
#include "cm_idle_recheck_cases.h"
#include "cm_super_dispatch_cases.h"
#include "cm_svct3_cases.h"
static const uint16_t start[CM_EXI_COHORTS+1]={0,1024,2048,2560,3584,5120};
enum cm_exi_cohort cm_exi_cohort(uint16_t n){unsigned k=0;while(k+1u<CM_EXI_COHORTS&&n>=start[k+1u])++k;return (enum cm_exi_cohort)k;}
uint16_t cm_exi_first(enum cm_exi_cohort k){return start[k];}
void cm_exi_input(apollo_overlay_state *m,cm_native_io *io,uint16_t n){enum cm_exi_cohort k=cm_exi_cohort(n);uint32_t j=(uint32_t)(n-start[k]);
 switch(k){
 /* NEWJOB +0, a basic job, an interpretive job, -0 (self-check). */
 case CM_EXI_ADVAN:cm_idle_dispatch_input(m,io,(uint16_t)((j*4u+j/256u)%CM_IDLE_DISPATCH_CASES));break;
 case CM_EXI_NUCHANG2:cm_idle_recheck_input(m,io,(j*12u+j/86u)%CM_IDLE_RECHECK_CASES);break;
 /* DUMMYJOB (even) and DUMMYJOB+2 (odd) over the idle states, with SELFRET
  * anywhere in the self-check bank. */
 case CM_EXI_DUMMYJOB:cm_idle_dispatch_input(m,io,(uint16_t)((j*8u+j/64u)%CM_IDLE_DISPATCH_CASES));
  m->words[5]=(agc_word)(j&1u?CM_EXEC_DUMMYJOB+2u:CM_EXEC_DUMMYJOB);m->words[CM_EXEC_SELFRET]=(agc_word)(02000u+(j*0151u)%02000u);
  io->channel11=(agc_word)((j*0123u)&AGC_MASK);break;
 /* SUPDXCHZ and SUPDXCHZ+1 (the generator's cases below 28672). */
 case CM_EXI_SUPDXCHZ:cm_super_dispatch_input(m,io,(j*28u+j/37u)%28672u);break;
 default:cm_svct3_input(m,io,(uint16_t)j);break;
 }
}
unsigned cm_exi_run(apollo_overlay_state *s,cm_native_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&cm_exec_idle_routines;program.basic_count=1;
 cm_exi_input(s,io,n);
 call=agc_transfer_resolve(&program,s,io->superbank);
 if(!call)return 3u;
 if(call(&program,s,io)!=EXEC_OK)return CM_EXI_REFUSED;
 return s->words[5]==CM_EXEC_BAILOUT||s->words[5]==CM_EXEC_POODOO?CM_EXI_ALARM:CM_EXI_RETURNED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_exi_digest(apollo_overlay_state *s,uint16_t n){cm_native_io io;unsigned i,kind=cm_exi_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=CM_EXI_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.superbank);mix(io.interrupt_enabled);mix(io.channel11);}
 return (uint32_t)sum_b<<16|sum_a;
}
