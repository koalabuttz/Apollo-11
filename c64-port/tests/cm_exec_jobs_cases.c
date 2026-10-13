/* SPDX-License-Identifier: MIT */
#include "cm_exec_jobs_cases.h"
#include "cm_admission_zero_cases.h"
#include "cm_context_change_cases.h"
#include "cm_sleep_cases.h"
#include "cm_wake_complete_cases.h"
#include "cm_priority_complete_cases.h"
#include "cm_basic_finish_cases.h"
#define PER 1024u
enum cm_exj_cohort cm_exj_cohort(uint16_t n){return (enum cm_exj_cohort)(n/PER<CM_EXJ_COHORTS?n/PER:CM_EXJ_COHORTS-1u);}
uint16_t cm_exj_first(enum cm_exj_cohort k){return (uint16_t)(k*PER);}
/* A caller return inside EXECUTIVE.agc itself (fixed-fixed 05134-05234, or
 * 01,2626-01,3245) has no producer: no job calls these services from the
 * Executive's own code. The oracle could not tell such a return from the
 * Executive passing that address on its way, so those inputs are skipped. */
static int in_code(agc_word z,agc_word fb){z&=07777u;return (z>=05132u&&z<=05236u)||((fb&076000u)==02000u&&z>=02624u&&z<=03247u);}
/* The same holds for a saved job location (core set LOC and BANK). */
static int inside_executive(const apollo_overlay_state *m){uint8_t i;
 if(in_code(m->words[2],m->words[4]))return 1;
 for(i=0;i<7u;++i)if(in_code(m->words[0164u+12u*i],m->words[0165u+12u*i]))return 1;
 return 0;
}
/* Products are formed in 32 bits: the C64's int is 16-bit. */
static void pick(apollo_overlay_state *m,cm_native_io *io,uint16_t n,uint32_t j){
 switch(cm_exj_cohort(n)){
 /* SPVAC (kind 2) and SPVACIN (kind 3), every core occupancy, +0 core and VAC count. */
 case CM_EXJ_SPVAC:(void)cm_admission_zero_input(m,io,4u*((j/2u)*167u%21504u)+2u+(j&1u));break;
 /* CHANG1 (even) and CHANG2 (odd) for basic and interpretive callers and successors. */
 case CM_EXJ_CHANGE:cm_context_change_input(m,io,(j*12u+j/86u)%CM_CONTEXT_CHANGE_CASES);break;
 case CM_EXJ_JOBSLEEP:cm_sleep_input(m,io,(uint16_t)((j*16u+j/64u)%CM_SLEEP_CASES));break;
 case CM_EXJ_JOBWAKE:cm_wake_complete_input(m,io,(j*32u+j/32u)%CM_WAKE_COMPLETE_CASES);break;
 case CM_EXJ_PRIOCHNG:cm_priority_complete_input(m,io,(j*14u+j/73u)%CM_PRIORITY_COMPLETE_CASES);break;
 default:cm_basic_finish_input(m,io,(uint16_t)((j*16u+j/64u)%CM_BASIC_FINISH_CASES));break;
 }
}
void cm_exj_input(apollo_overlay_state *m,cm_native_io *io,uint16_t n){uint16_t j=(uint16_t)(n%PER),k;
 pick(m,io,n,j);
 for(k=1;inside_executive(m)&&k<64u;++k)pick(m,io,n,j+(uint32_t)k*PER);
}
unsigned cm_exj_run(apollo_overlay_state *s,cm_native_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&cm_exec_job_routines;program.basic_count=1;
 cm_exj_input(s,io,n);
 call=agc_transfer_resolve(&program,s,io->superbank);
 if(!call)return 3u;
 if(call(&program,s,io)!=EXEC_OK)return CM_EXJ_REFUSED;
 return s->words[5]==CM_EXEC_BAILOUT||s->words[5]==CM_EXEC_POODOO?CM_EXJ_ALARM:CM_EXJ_RETURNED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_exj_digest(apollo_overlay_state *s,uint16_t n){cm_native_io io;unsigned i,kind=cm_exj_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=CM_EXJ_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.superbank);mix(io.interrupt_enabled);mix(io.channel11);}
 return (uint32_t)sum_b<<16|sum_a;
}
