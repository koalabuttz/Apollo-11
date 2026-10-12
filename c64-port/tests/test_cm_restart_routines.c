/* SPDX-License-Identifier: MIT */
/* cm-restart-routines: Comanche055 RESTARTS selection, RESTARTS-to-SWRETURN
 * composition and NEWPHASE from its callers, through the trampoline, against
 * digests frozen from the original program; every restart table entry
 * reaches its service with its 2CADR; every NEWPHASE site is entered; words
 * RESTARTS never stores, and A or Q no caller supplies, are refused. */
#include <stdio.h>
#include "cm_restart_routines_cases.h"
#include "cm_restart_routines_vectors.h"
#define CM_RESTART_TABLE
#include "../src/cm_restart_table_data.inc"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static int table_case(unsigned n){return n>=1024u&&(n<2048u||n>=3072u)&&n<6144u;}
int main(void){apollo_overlay_state s;unsigned n,p,seen[CM_RESTART_TABLE_WORDS]={0},entries=0,labelled=0,sites[CM_NEWPHASE_SITE_COUNT]={0},site_count=0;enum exec_status st;
 check(CM_RR_SELECT_VECTORS==CM_RR_SELECT_CASES&&CM_RR_COMPOSE_VECTORS==CM_RR_COMPOSE_CASES&&CM_RR_NEWPHASE_VECTORS==CM_RR_NEWPHASE_CASES,"vector counts",0);
 check(CM_RESTART_OPERAND_COUNT==2u+CM_NEWPHASE_SITE_COUNT,"operand count",0);
 for(n=1;n<CM_RESTART_OPERAND_COUNT;++n)check(cm_restart_operands[n-1u].physical<cm_restart_operands[n].physical,"operands sorted",n);
 for(n=1;n<cm_restart_routines.count;++n)check(cm_restart_routines.entries[n-1u].physical<cm_restart_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<CM_RR_SELECT_CASES;++n){check(cm_rr_select_digest(&s,(uint16_t)n)==cm_rr_select_expected[n],"selection",n);
  /* Table cohorts: the entry this selection read. */
  if(table_case(n)){p=s.words[0156];if(p<CM_RESTART_TABLE_WORDS&&!seen[p]){seen[p]=1;++entries;}}}
 /* Distinct table entries: each labelled phase's entry (aliases share
  * 4.2SPOT), and the second entry of each even phase, read by PHSPART2. */
 {unsigned k;uint8_t g;agc_word ph;unsigned char want[CM_RESTART_TABLE_WORDS]={0};
  for(k=0;k<cm_rr_pairs();++k){cm_rr_pair((uint16_t)k,&g,&ph);
   if(ph&1u)want[cm_restart_table_words[g*2u+1u]+ph+(ph>>1)]=1;
   else{want[cm_restart_table_words[g*2u]+3u*ph]=1;want[cm_restart_table_words[g*2u]+3u*ph+3u]=1;}}
  for(k=0;k<CM_RESTART_TABLE_WORDS;++k){labelled+=want[k];check(want[k]==seen[k],"table entry",k);}}
 check(entries==labelled,"every table entry selected",entries);
 for(n=0;n<CM_RR_COMPOSE_CASES;++n){cm_rr_log log;uint16_t steps;agc_word code;unsigned sel=cm_rr_compose_case((uint16_t)n);
  check(cm_rr_compose_digest(&s,(uint16_t)n,&st)==cm_rr_compose_expected[n]&&st==EXEC_OK,"composition",n);
  if(cm_rr_compose(&s,(uint16_t)n,&log,&steps)!=EXEC_OK||log.calls==0u||!table_case(sel))continue;
  /* The last service of a table phase receives the entry's CADR at GOLOC;
   * an even phase's first part is the entry three words earlier. */
  p=s.words[0156];code=cm_restart_table_words[p+1u];
  check(log.call[log.calls-1u].cadr==((code&AGC_SIGN)?agc_negate(code):code),"table 2CADR",n);
  if(log.calls==2u&&!(s.words[0154]&01400u)){code=cm_restart_table_words[p-2u];
   check(log.call[0].cadr==((code&AGC_SIGN)?agc_negate(code):code),"first-part 2CADR",n);}
 }
 for(n=0;n<CM_RR_NEWPHASE_CASES;++n){uint16_t at;
  check(cm_rr_newphase_digest(&s,(uint16_t)n,&st)==cm_rr_newphase_expected[n]&&st==EXEC_OK,"NEWPHASE",n);
  {cm_native_io io;cm_rr_newphase_input(&s,&io,(uint16_t)n);}at=agc_fixed_physical(s.words[2],s.words[4],0);
  for(p=0;p<CM_NEWPHASE_SITE_COUNT;++p)if(cm_restart_operands[2u+p].physical==at&&!sites[p]){sites[p]=1;++site_count;}}
 check(site_count==CM_NEWPHASE_SITE_COUNT,"every NEWPHASE site",site_count);
 {agc_native_program program={&cm_restart_routines,1,0,0,cm_restart_operands,CM_RESTART_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_word sb=0;uint16_t steps;cm_native_io io;
  /* Unmapped service: the trampoline stops at the selected service. */
  for(n=0;n<CM_RR_COMPOSE_CASES;n+=97u){(void)cm_rr_select_input(&s,cm_rr_compose_case((uint16_t)n));
   st=agc_transfer_run(&program,&s,0,&sb,FF(CM_RR_SWRETURN),8,&steps);
   check(st==EXEC_UNMAPPED&&(s.words[5]==CM_RR_FINDVAC||s.words[5]==CM_RR_NOVAC||s.words[5]==CM_RR_WAITLIST||s.words[5]==CM_RR_LONGCALL),"unmapped service",n);}
  /* Words RESTARTS never stores at GOLOC-1/GOLOC+2 are out of scope. */
  (void)cm_rr_select_input(&s,0);s.words[5]=0704;s.words[0704]=05000;
  check(agc_transfer_run(&program,&s,0,&sb,FF(CM_RR_SWRETURN),8,&steps)==EXEC_OUT_OF_SCOPE,"GOLOC-1 refusal",0);
  (void)cm_rr_select_input(&s,0);s.words[5]=0707;s.words[0707]=03520;
  check(agc_transfer_run(&program,&s,0,&sb,FF(CM_RR_SWRETURN),8,&steps)==EXEC_OUT_OF_SCOPE,"GOLOC+2 refusal",0);
  (void)cm_rr_select_input(&s,0);s.words[6]=02002;
  check(cm_restart_routines_select(&s,CM_RR_GROUP)==CM_RR_INVALID,"bank refusal",0);
  /* NEWPHASE from a Q that is no `TC NEWPHASE` site, or with an overflowed A. */
  cm_rr_newphase_input(&s,&io,0);s.words[2]=(agc_word)(s.words[2]+1u);
  check(agc_transfer_run(&program,&s,&io,&sb,FF(CM_RR_SWRETURN),2,&steps)==EXEC_OUT_OF_SCOPE,"NEWPHASE site refusal",0);
  cm_rr_newphase_input(&s,&io,0);s.words[0]=0100003;
  check(agc_transfer_run(&program,&s,&io,&sb,FF(CM_RR_SWRETURN),2,&steps)==EXEC_OUT_OF_SCOPE,"NEWPHASE overflow refusal",0);
  cm_rr_newphase_input(&s,&io,0);
  check(agc_transfer_run(&program,&s,0,&sb,FF(CM_RR_SWRETURN),2,&steps)==EXEC_OUT_OF_SCOPE,"NEWPHASE without io",0);}
 printf("cm-restart-routines: %u selection, %u composition and %u NEWPHASE cases, %u table entries, %u NEWPHASE sites, %u checks, %u failures\n",
        CM_RR_SELECT_CASES,CM_RR_COMPOSE_CASES,CM_RR_NEWPHASE_CASES,entries,site_count,checks,failures);
 return failures!=0u;
}
