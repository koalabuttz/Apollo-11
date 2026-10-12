/* SPDX-License-Identifier: MIT */
/* lm-exec-jobs: Luminary099 SPVAC, CHANG1/CHANG2, JOBSLEEP, JOBWAKE, PRIOCHNG,
 * ENDOFJOB, FIXDELAY, VARDELAY, TASKOVER and the T3 vector as native routines,
 * against digests frozen from the original program; every entry, alarm and
 * successor kind is reached; inputs no original producer supplies are refused
 * with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_exec_jobs_cases.h"
#include "lm_exec_jobs_vectors.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static const uint16_t points[12]={FF(LM_EXEC_T3RUPT_VECTOR),FF(LM_EXEC_SPVACIN),FF(LM_EXEC_SPVAC),FF(LM_EXEC_CHANG1),FF(LM_EXEC_CHANG2),
 FF(LM_EXEC_JOBSLEEP),FF(LM_EXEC_JOBWAKE),FF(LM_EXEC_PRIOCHNG),FF(LM_EXEC_ENDOFJOB),FF(LM_EXEC_FIXDELAY),FF(LM_EXEC_VARDELAY),FF(LM_EXEC_TASKOVER)};
static unsigned reached[12],alarms[4],ends[LM_EXJ_COHORTS][3],irq[2];
static agc_routine_table tables[2];
static agc_native_program jobs_program(void){agc_native_program p={tables,2,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
 tables[0]=lm_exec_job_routines;tables[1]=lm_exec_task_routines;p.operands=lm_exec_site_table(&p.operand_count);return p;
}
/* The trampoline loop, recording each entry point a case passes. */
static void cover(uint16_t n){apollo_overlay_state s;p66_bias_io io;lm_exj_case c;uint16_t point,stop;unsigned i,k;agc_routine call;
 agc_native_program program=jobs_program();
 lm_exj_input(&s,&io,&c,n);if(lm_exj_cohort(n)==LM_EXJ_DISPATCH)++irq[io.interrupt_enabled&1u];program.hardware=&c.hw;stop=c.until?FF(c.until):AGC_TRANSFER_NONE;
 for(k=0;k<8u;++k){point=agc_transfer_point(&program,&s,io.superbank);if(point==stop)return;
  call=agc_transfer_resolve(&program,&s,io.superbank);if(!call)return;
  for(i=0;i<12u;++i)if(points[i]==point)++reached[i];
  if(call(&program,&s,&io)!=EXEC_OK)return;}
}
/* A refusal leaves the image, io and hardware as they were. */
static void refused(uint16_t n,void (*edit)(apollo_overlay_state *,p66_bias_io *,lm_exj_case *),int hardware,const char *what){
 apollo_overlay_state s,before;p66_bias_io io,old;lm_exj_case c;agc_hardware hw;uint16_t steps;
 agc_native_program program=jobs_program();
 lm_exj_input(&s,&io,&c,n);edit(&s,&io,&c);hw=c.hw;program.hardware=hardware?&c.hw:0;before=s;old=io;
 check(agc_transfer_run(&program,&s,&io,&io.superbank,AGC_TRANSFER_NONE,8,&steps)==EXEC_OUT_OF_SCOPE&&steps==1u&&
  !memcmp(&s,&before,sizeof s)&&!memcmp(&io,&old,sizeof io)&&!memcmp(&hw,&c.hw,sizeof hw),what,n);
}
static void as_is(apollo_overlay_state *s,p66_bias_io *io,lm_exj_case *c){(void)s;(void)io;(void)c;}
static void off_site(apollo_overlay_state *s,p66_bias_io *io,lm_exj_case *c){(void)io;(void)c;s->words[2]=(agc_word)(s->words[2]+1u);}
/* Every interpretive LOC (cores of one parity) beyond the 07777 INTRSM range. */
static void bad_loc(apollo_overlay_state *s,p66_bias_io *io,lm_exj_case *c){uint8_t i;(void)io;(void)c;
 for(i=1;i<8u;++i)if(s->words[0164u+12u*i]&AGC_SIGN)s->words[0164u+12u*i]=agc_negate(07777u);}
static uint16_t first_with(enum lm_exj_cohort k,agc_word z){uint16_t n;apollo_overlay_state s;p66_bias_io io;lm_exj_case c;uint16_t steps;
 for(n=0;n<LM_EXJ_CASES;++n)if(lm_exj_cohort(n)==k&&lm_exj_run(&s,&io,&c,n,&steps)==LM_EXJ_RETURNED&&s.words[5]==z)return n;
 return 0;
}
int main(void){apollo_overlay_state s;p66_bias_io io;lm_exj_case c;unsigned n,i,kind[3]={0,0,0},interp[2]={0,0},idle[2]={0,0},resumed=0;uint16_t steps,m;
 check(LM_EXJ_VECTORS==LM_EXJ_CASES,"vector count",LM_EXJ_VECTORS);
 for(n=0;n<LM_EXJ_CASES;++n){enum lm_exj_cohort k=lm_exj_cohort((uint16_t)n);unsigned r=lm_exj_run(&s,&io,&c,(uint16_t)n,&steps);
  check(lm_exj_digest(&s,(uint16_t)n)==lm_exj_expected[n],"digest",n);
  check(r==LM_EXJ_RETURNED||r==LM_EXJ_ALARM,"outcome",n);if(r<3u){++kind[r];++ends[k][r];}
  if(r==LM_EXJ_ALARM){if(s.words[5]==LM_EXEC_POODOO1)++alarms[3];else ++alarms[s.words[2]==02617u?0:s.words[2]==02703u?1:2];}
  if(r==LM_EXJ_RETURNED&&(k==LM_EXJ_JOBSLEEP||k==LM_EXJ_PRIOCHNG)&&s.words[5]==LM_EXEC_NEWOPS)++interp[k==LM_EXJ_PRIOCHNG];
  if(r==LM_EXJ_RETURNED&&(k==LM_EXJ_JOBSLEEP||k==LM_EXJ_ENDOFJOB)&&s.words[5]==LM_EXEC_ADVAN)++idle[k==LM_EXJ_ENDOFJOB];
  if(r==LM_EXJ_RETURNED&&k==LM_EXJ_RESUME&&c.hw.in_interrupt==0u)++resumed;
  cover((uint16_t)n);}
 for(i=0;i<12u;++i)check(reached[i]>0u,"entry reached",i);
 for(i=0;i<4u;++i)check(alarms[i]>0u,"alarm reached",i);
 check(interp[0]&&interp[1],"interpretive successor after JOBSLEEP and PRIOCHNG",0);
 check(idle[0]&&idle[1],"idle after JOBSLEEP and ENDOFJOB",0);
 check(resumed==ends[LM_EXJ_RESUME][0]&&resumed>0u,"RESUME ends the interrupt",resumed);
 check(irq[0]&&irq[1],"T3 dispatch with interrupts inhibited and enabled",0);
 check(reached[11]>ends[LM_EXJ_RESUME][0],"TASKOVER dispatching the next task",reached[11]);
 /* Inputs no producer supplies. */
 refused(lm_exj_first(LM_EXJ_JOBSLEEP),as_is,0,"JOBSLEEP without hardware");
 refused(lm_exj_first(LM_EXJ_ENDOFJOB),as_is,0,"ENDOFJOB without hardware");
 refused(lm_exj_first(LM_EXJ_RESUME),as_is,0,"TASKOVER without hardware");
 refused(lm_exj_first(LM_EXJ_DISPATCH),as_is,0,"T3 vector without hardware");
 refused(lm_exj_first(LM_EXJ_FIXDELAY_SITES),off_site,1,"FIXDELAY Q off a call site");
 m=first_with(LM_EXJ_JOBSLEEP,LM_EXEC_NEWOPS);check(m!=0u,"a JOBSLEEP interpretive case",0);refused(m,bad_loc,1,"JOBSLEEP interpretive LOC");
 m=first_with(LM_EXJ_PRIOCHNG,LM_EXEC_NEWOPS);check(m!=0u,"a PRIOCHNG interpretive case",0);refused(m,bad_loc,1,"PRIOCHNG interpretive LOC");
 printf("lm-exec-jobs: %u cases (%u returned, %u alarm, %u refused); entries",LM_EXJ_CASES,kind[0],kind[1],kind[2]);
 for(i=0;i<12u;++i)printf(" %u",reached[i]);
 printf("; alarms 1201 %u 1202 %u 1203 %u 1204 %u; interpretive %u/%u; idle %u/%u; resumed %u; %u checks, %u failures\n",
  alarms[0],alarms[1],alarms[2],alarms[3],interp[0],interp[1],idle[0],idle[1],resumed,checks,failures);
 return failures!=0u;
}
