/* SPDX-License-Identifier: MIT */
/* lm-restart-select: Luminary099 RESTARTS selection and RESTARTS-to-SWRETURN
 * composition through the trampoline, against digests frozen from the
 * original program; every restart table entry reaches its service with its
 * 2CADR; words RESTARTS never stores are refused. */
#include <stdio.h>
#include "lm_restart_routines_cases.h"
#include "lm_restart_vectors.h"
#include "../src/lm_restart_table_data.inc"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
int main(void){apollo_overlay_state s;unsigned n,p,seen[LM_RESTART_TABLE_WORDS]={0},entries=0;enum exec_status st;
 for(n=0;n<LM_RSSELECT_CASES;++n){check(lm_rs_select_digest(&s,(uint16_t)n)==lm_rs_select_expected[n],"selection",n);
  /* Table cohorts: the entry this selection read. */
  if(n>=1024u&&(n<2048u||n>=3072u)&&n<6144u){p=s.words[0156];if(p>=LM_RESTART_TABLE_FIRST&&p-LM_RESTART_TABLE_FIRST<LM_RESTART_TABLE_WORDS&&!seen[p-LM_RESTART_TABLE_FIRST]){seen[p-LM_RESTART_TABLE_FIRST]=1;++entries;}}}
 for(n=0;n<LM_RSR_CASES;++n){lm_rsr_log log;uint16_t steps;agc_word code;
  check(lm_rs_compose_digest(&s,(uint16_t)n,&st)==lm_rs_compose_expected[n]&&st==EXEC_OK,"composition",n);
  if(lm_rsr_run(&s,(uint16_t)n,&log,&steps)!=EXEC_OK||log.calls==0u)continue;
  if(lm_rsr_select_case((uint16_t)n)<1024u||(lm_rsr_select_case((uint16_t)n)>=2048u&&lm_rsr_select_case((uint16_t)n)<3072u))continue;
  /* The last service of a table phase receives the entry's CADR at GOLOC. */
  /* An even phase's first part is the entry three words earlier. */
  p=s.words[0156];code=lm_restart_table_data[p+1u-LM_RESTART_TABLE_FIRST];
  check(log.call[log.calls-1u].cadr==((code&AGC_SIGN)?agc_negate(code):code),"table 2CADR",n);
  if(log.calls==2u&&!(s.words[0154]&01400u)){code=lm_restart_table_data[p-2u-LM_RESTART_TABLE_FIRST];
   check(log.call[0].cadr==((code&AGC_SIGN)?agc_negate(code):code),"first-part 2CADR",n);}
 }
 check(entries==40u,"every table entry selected",entries);
 /* Unmapped service: the trampoline stops at the selected service. */
 {agc_native_program program={&lm_restart_routines,1,0,0,lm_restart_operands,2,AGC_TRANSFER_NONE,0,0,0,0,0};agc_word sb=0;uint16_t steps;
  for(n=0;n<LM_RSR_CASES;n+=97u){(void)lm_rsselect_input(&s,lm_rsr_select_case((uint16_t)n));
   st=agc_transfer_run(&program,&s,0,&sb,FF(LM_RS_SWRETURN),8,&steps);
   check(st==EXEC_UNMAPPED&&(s.words[5]==LM_RS_FINDVAC||s.words[5]==LM_RS_NOVAC||s.words[5]==LM_RS_WAITLIST||s.words[5]==LM_RS_LONGCALL),"unmapped service",n);}
  /* Words RESTARTS never stores at GOLOC-1/GOLOC+2 are out of scope. */
  (void)lm_rsselect_input(&s,0);s.words[5]=0704;s.words[0704]=05000;
  check(agc_transfer_run(&program,&s,0,&sb,FF(LM_RS_SWRETURN),8,&steps)==EXEC_OUT_OF_SCOPE,"GOLOC-1 refusal",0);
  (void)lm_rsselect_input(&s,0);s.words[5]=0707;s.words[0707]=03526;
  check(agc_transfer_run(&program,&s,0,&sb,FF(LM_RS_SWRETURN),8,&steps)==EXEC_OUT_OF_SCOPE,"GOLOC+2 refusal",0);
  (void)lm_rsselect_input(&s,0);s.words[6]=02002;
  check(lm_restart_select(&s,LM_RS_SELECT_GROUP)==LM_RS_SELECT_INVALID,"bank refusal",0);}
 printf("lm-restart-select: %u selection and %u composition cases, %u table entries, %u checks, %u failures\n",LM_RSSELECT_CASES,LM_RSR_CASES,entries,checks,failures);
 return failures!=0u;
}
