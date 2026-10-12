/* SPDX-License-Identifier: MIT */
#include "lm_exec_jobs_cases.h"
/* LM_EXJ_PART 1, 2 or 3 builds only the admission and change, the switch,
 * or the task cohorts (the C64 programs). */
#ifndef LM_EXJ_PART
#define LM_EXJ_CHANGES 1
#define LM_EXJ_SWITCHES 1
#define LM_EXJ_TASKS 1
#else
#define LM_EXJ_CHANGES (LM_EXJ_PART==1)
#define LM_EXJ_SWITCHES (LM_EXJ_PART==2)
#define LM_EXJ_TASKS (LM_EXJ_PART==3)
#endif
#define LM_EXJ_JOBS (LM_EXJ_CHANGES||LM_EXJ_SWITCHES)
#include "lm_admission_image_cases.h"
#include "lm_basic_change_cases.h"
#include "lm_interpreter_change_cases.h"
#include "lm_sleep_cases.h"
#include "lm_wake_cases.h"
#include "lm_priority_cases.h"
#include "lm_finish_cases.h"
#include "lm_delay_image_cases.h"
#include "lm_taskover_resume_cases.h"
#include "lm_waitlist_dispatch_cases.h"
#if LM_EXJ_TASKS
#define AGC_SITES_CALLEES
#include "../src/lm_exec_sites.inc"
#endif
static const uint16_t first[LM_EXJ_COHORTS+1]={0,1024,1792,2560,3584,4096,5120,6144,7168,7680,8192,9216};
enum lm_exj_cohort lm_exj_cohort(uint16_t n){uint8_t k=0;while(k+1u<LM_EXJ_COHORTS&&n>=first[k+1u])++k;return (enum lm_exj_cohort)k;}
uint16_t lm_exj_first(enum lm_exj_cohort k){return first[k];}
static void stop(lm_exj_case *c,agc_word z){if(c->stops<LM_EXJ_MAX_STOPS)c->stop[c->stops++]=z;}
#if LM_EXJ_CHANGES
/* The core set CHANJOB switches to (NEWJOB): its basic entry, or NEWOPS. */
static void target(const apollo_overlay_state *m,lm_exj_case *c){agc_word loc=m->words[0164u+m->words[067]];stop(c,(loc&AGC_SIGN)?LM_EXEC_NEWOPS:loc);}
#endif
#if LM_EXJ_SWITCHES
static void jobs(lm_exj_case *c){agc_word z;stop(c,LM_EXEC_ADVAN);stop(c,LM_EXEC_NEWOPS);for(z=02001;z<=02007u;++z)stop(c,z);}
/* Odd or even core sets 1-7 hold interpretive jobs (negative LOC). */
static void interpretive(apollo_overlay_state *m,uint16_t j){uint8_t i;
 for(i=1;i<8u;++i)if(i%2u==(j/2u)%2u)m->words[0164u+12u*i]=agc_negate((agc_word)(03360u+i));
}
#endif
#if LM_EXJ_TASKS
/* Genuine FIXDELAY site k (0..15): index in lm_exec_sites. */
static uint16_t fixdelay_site(uint16_t k){uint16_t i,seen=0;
 for(i=0;i<LM_EXEC_SITES_COUNT;++i)if(lm_exec_sites_callee[i]==LM_EXEC_FIXDELAY){if(seen==k)return i;++seen;}
 return 0;
}
#endif
void lm_exj_input(apollo_overlay_state *m,p66_bias_io *io,lm_exj_case *c,uint16_t n){
 enum lm_exj_cohort k=lm_exj_cohort(n);uint16_t j=(uint16_t)(n-first[k]);
 c->stops=0;c->resume=0;c->until=0;c->hw.channel11=0;c->hw.in_interrupt=0;
 switch(k){
#if LM_EXJ_CHANGES
 case LM_EXJ_SPVAC:
  (void)lm_admission_image_input(m,io,(uint16_t)(2u+(j&1u)+4u*((j*37u)%4096u)));
  /* SPVAC returns to the caller's Q, SPVACIN to Q+2. */
  stop(c,m->words[2]);stop(c,(agc_word)(m->words[2]+2u));
  break;
 case LM_EXJ_CHANG1:
  lm_basic_change_input(m,io,(uint16_t)(2u*((j*29u)%8192u)));target(m,c);break;
 case LM_EXJ_CHANG2:
  lm_interpreter_change_input(m,io,(uint16_t)((j*21u)%16384u));target(m,c);break;
#endif
#if LM_EXJ_SWITCHES
 case LM_EXJ_JOBSLEEP:
  lm_sleep_input(m,io,&c->hw.channel11,(uint16_t)((j*13u+5u)%8192u));
  if(j&1u)interpretive(m,j);
  jobs(c);break;
 case LM_EXJ_JOBWAKE:
  lm_wake_input(m,io,(uint16_t)((j*16u)%8192u));stop(c,m->words[2]);
  break;
 case LM_EXJ_PRIOCHNG:
  lm_priority_input(m,io,(uint16_t)((j*8u+j/1024u)%8192u));
  if(j&1u)interpretive(m,j);
  stop(c,m->words[2]);jobs(c);break;
 case LM_EXJ_ENDOFJOB:
  lm_finish_input(m,io,&c->hw.channel11,(uint16_t)((j*13u)%8192u+(j&1u)*8192u));jobs(c);break;
#endif
#if LM_EXJ_TASKS
 case LM_EXJ_DELAY:
  /* FIXDELAY (delay in erasable at Q) or VARDELAY from a task. The Waitlist
   * input domain is taken below 7560, where its (n/20)*173 still fits the
   * C64's 16-bit unsigned int. */
  (void)lm_delay_image_input(m,io,(uint16_t)(6u*((j*7u)%1260u)+3u*((j>>1)&1u)+(j&1u)));
  c->hw.in_interrupt=1;c->until=LM_EXEC_TASKOVER;stop(c,LM_EXEC_TASKOVER);break;
 case LM_EXJ_FIXDELAY_SITES:{
  uint16_t e=fixdelay_site(j%16u),v=(uint16_t)(j/16u),ph=lm_exec_sites[e].physical,bank=(uint16_t)(ph/1024u),off=(uint16_t)(ph%1024u);agc_word fb,eb;
  (void)lm_delay_image_input(m,io,(uint16_t)(6u*((v*97u+j*13u)%1260u)));
  eb=m->words[3];
  if(bank==2u||bank==3u){m->words[2]=(agc_word)(bank*02000u+off);fb=(agc_word)(((v*5u)%32u)*02000u);io->superbank=(agc_word)((v%4u)*020u);}
  else if(bank>=040u){m->words[2]=(agc_word)(02000u+off);fb=(agc_word)((bank-8u)*02000u);io->superbank=0100;}
  else{m->words[2]=(agc_word)(02000u+off);fb=(agc_word)(bank*02000u);io->superbank=(agc_word)((v%4u)*020u);}
  m->words[4]=fb;m->words[6]=(agc_word)(fb|(eb>>8));m->words[5]=LM_EXEC_FIXDELAY;
  c->hw.in_interrupt=1;c->until=LM_EXEC_TASKOVER;stop(c,LM_EXEC_TASKOVER);break;}
 case LM_EXJ_RESUME:{agc_word c14;
  lm_taskover_resume_input(m,io,&c->hw.channel11,&c14,(uint16_t)((j*16u+j/512u)%8192u));
  c->hw.in_interrupt=1;c->resume=1;stop(c,(agc_word)(m->words[015]-1u));break;}
 case LM_EXJ_DISPATCH:{agc_word c14;uint8_t i;
  /* The T3 vector or TASKOVER with RUPTAGN +1, either interrupt-enable state. */
  lm_waitlist_dispatch_input(m,io,&c->hw.channel11,&c14,(uint16_t)((j*8u+(j&1u))%8192u));
  io->interrupt_enabled=(uint16_t)((j/2u)%2u);c->hw.in_interrupt=1;
  for(i=0;i<9u;++i)stop(c,m->words[01410u+2u*i]);
  break;}
#endif
 default:break;
 }
}
unsigned lm_exj_run(apollo_overlay_state *s,p66_bias_io *io,lm_exj_case *c,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;uint8_t i;agc_word z;
#if LM_EXJ_JOBS
 tables[program.basic_count++]=lm_exec_job_routines;
#endif
#if LM_EXJ_TASKS
 tables[program.basic_count++]=lm_exec_task_routines;
#endif
 program.basic=tables;
#if LM_EXJ_TASKS
 program.operands=lm_exec_site_table(&program.operand_count);
#endif
 lm_exj_input(s,io,c,n);program.hardware=&c->hw;
 st=agc_transfer_run(&program,s,io,&io->superbank,c->until?agc_fixed_physical(c->until,0,0):AGC_TRANSFER_NONE,8,steps);
 if(st==EXEC_OUT_OF_SCOPE)return LM_EXJ_REFUSED;
 if(st==EXEC_OK)return LM_EXJ_RETURNED;
 if(st!=EXEC_UNMAPPED)return 3u;
 z=s->words[5];
 if(z==LM_EXEC_BAILOUT1||z==LM_EXEC_POODOO1)return LM_EXJ_ALARM;
 for(i=0;i<c->stops;++i)if(z==c->stop[i])return LM_EXJ_RETURNED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_exj_digest(apollo_overlay_state *s,uint16_t n){p66_bias_io io;lm_exj_case c;uint16_t steps;unsigned i,kind=lm_exj_run(s,&io,&c,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LM_EXJ_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.superbank);mix(io.interrupt_enabled);mix(c.hw.channel11);mix(c.hw.in_interrupt);}
 return (uint32_t)sum_b<<16|sum_a;
}
