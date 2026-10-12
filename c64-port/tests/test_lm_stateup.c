/* SPDX-License-Identifier: MIT */
/* lm-stateup: Luminary099 FRESH_START_AND_RESTART bank 13 (STATEUP, DOINT,
 * NO-INT, DOINT2, THISVINT) cases through the trampoline against digests
 * frozen from the original program; every order pair is reached, each run
 * ends at NEWOPS of INTEGRV, INTSTALL, SETIFLGS, ENDINT or THISVINT's
 * caller, or at CHANG2, and every one of those is reached; LOC on an
 * operand word, BBANK not under BANKSET, a work area outside the VAC
 * areas, an erasable QPRET at THISVINT and a missing io are refused. */
#include <stdio.h>
#include "lm_stateup_cases.h"
#include "lm_stateup_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[LM_STATEUP_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((p66_bias_io *)io)->superbank);
 for(i=0;i<LM_STATEUP_ROUTINES;++i)if(lm_stateup_interpretive.entries[i].physical==at){++reached[i];return lm_stateup_interpretive.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static agc_native_program program(const agc_routine_table *t){
 agc_native_program p={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
 p.interpretive=t;p.interpretive_count=1;p.interpreter_fetch=(uint16_t)(2048u+LM_STATEUP_NEWOPS-04000u);
 p.loc=LM_STATEUP_LOC;p.bankset=LM_STATEUP_BANKSET;return p;
}
static enum exec_status once(apollo_overlay_state *s,p66_bias_io *io){
 agc_native_program p=program(&lm_stateup_interpretive);agc_routine call=agc_transfer_resolve(&p,s,io?io->superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
static void at(apollo_overlay_state *s,p66_bias_io *io,agc_word loc){unsigned k;
 for(k=0;k<LM_STATEUP_ROUTINES&&lm_stateup_entries[k]!=loc;++k){}
 lm_stateup_input(s,io,(uint16_t)k);s->words[067]=0;
}
/* Where a run may end: NEWOPS with LOC (bank 13), or Z (CHANG2). */
static const struct { uint16_t z,loc; const char *name; } ends[]={
 {06070u,03134u,"INTEGRV"},{06070u,03414u,"INTSTALL"},{06070u,02644u,"SETIFLGS"},{06070u,02632u,"ENDINT"},
 {06070u,03603u,"THISVINT's caller"},{06070u,0u,"another bank 13 return"},{05126u,0u,"CHANG2"}};
#define ENDS (sizeof ends/sizeof ends[0])
int main(void){apollo_overlay_state s;p66_bias_io io;enum exec_status st;unsigned n,i,ended[ENDS];
 for(i=0;i<ENDS;++i)ended[i]=0;
 check(LM_STATEUP_VECTORS==LM_STATEUP_CASES,"vector count",0);
 for(n=1;n<LM_STATEUP_ROUTINES;++n)check(lm_stateup_interpretive.entries[n-1u].physical<lm_stateup_interpretive.entries[n].physical,"routines sorted",n);
 for(n=0;n<LM_STATEUP_CASES;++n)check(lm_stateup_digest(&s,(uint16_t)n,&st)==lm_stateup_expected[n]&&st==EXEC_OK,"oracle",n);
 {agc_routine_entry wrapped[LM_STATEUP_ROUTINES];agc_routine_table t;agc_native_program p;uint16_t steps;
  for(i=0;i<LM_STATEUP_ROUTINES;++i){wrapped[i].physical=lm_stateup_interpretive.entries[i].physical;wrapped[i].call=count;}
  t.entries=wrapped;t.count=LM_STATEUP_ROUTINES;p=program(&t);
  for(n=0;n<LM_STATEUP_CASES;++n){agc_word z,loc;
   lm_stateup_input(&s,&io,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.superbank,AGC_TRANSFER_NONE,32,&steps);
   check(st==EXEC_UNMAPPED,"coverage run",n);
   z=s.words[5];loc=s.words[LM_STATEUP_LOC];
   /* CALL, STCALL and CLRGO targets are bank 13 CADRs. */
   if(z==06070u&&(loc==03134u||loc==03414u||loc==02644u||loc==02632u))check((s.words[LM_STATEUP_BANKSET]&076000u)==(013u<<10),"target in bank 13",n);
   for(i=0;i<ENDS;++i)if(ends[i].z==z&&(ends[i].loc==0u||ends[i].loc==loc))break;
   if(i<ENDS)++ended[i];else check(0,"end point",n);}
  for(i=0;i<LM_STATEUP_ROUTINES;++i)check(reached[i]>0u,"pair reached",i);
  for(i=0;i<ENDS;++i)check(ended[i]>0u,ends[i].name,i);}
 /* Refusals, each entered once directly. */
 at(&s,&io,02026u);s.words[LM_STATEUP_LOC]=02027u;check(once(&s,&io)==EXEC_UNMAPPED,"LOC on an operand word is no entry",0);
 at(&s,&io,02026u);s.words[LM_STATEUP_BANKSET]=(agc_word)(014u<<10);check(once(&s,&io)!=EXEC_OK,"BBANK not under BANKSET",0);
 at(&s,&io,02041u);s.words[0120]=0402u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"work area outside the VAC areas",0);
 at(&s,&io,02066u);s.words[s.words[0120]+052u]=01000u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"erasable QPRET at RVQ",0);
 at(&s,&io,02026u);check(once(&s,0)==EXEC_OUT_OF_SCOPE,"no io",0);
 printf("lm-stateup: %u cases, %u pairs, ends",LM_STATEUP_CASES,LM_STATEUP_ROUTINES);for(i=0;i<ENDS;++i)printf(" %u",ended[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
