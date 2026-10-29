/* SPDX-License-Identifier: MIT */
/* lm-update: Luminary099 UPDATE_PROGRAM cases through the trampoline, against
 * digests frozen from the original program; every registered entry is
 * reached (UPJOB's interpretive words too) and each run ends at an entry
 * outside the file; a TC UPTEMP or
 * INDEX UPTEMP; TC 1 word TIMEDIDL never stores, an UPVERB with no branch,
 * an update that would write a register or counter, a missing io record and
 * UPACTOFF without the hardware record are refused. */
#include <stdio.h>
#include "lm_update_cases.h"
#include "lm_update_vectors.h"
#include "lm_update_c64_vectors.h"
#include "lm_interbank_routines.h"
#include "../src/lm_update_operands.inc"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[LM_UP_ROUTINES],ireached[LM_UP_INTERPRETIVE];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((p66_bias_io *)io)->superbank);
 for(i=0;i<LM_UP_ROUTINES;++i)if(lm_update_routines.entries[i].physical==at){++reached[i];return lm_update_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status icount(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_fixed_physical(s->words[LM_UP_LOC],s->words[LM_UP_BANKSET],((p66_bias_io *)io)->superbank);
 for(i=0;i<LM_UP_INTERPRETIVE;++i)if(lm_update_interpretive.entries[i].physical==at){++ireached[i];return lm_update_interpretive.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw){
 agc_native_program p={&lm_update_routines,1,0,0,lm_update_operands,LM_UPDATE_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.hardware=hw;call=agc_transfer_resolve(&p,s,io?io->superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The first case entering at z. */
static void at(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,agc_word z){unsigned k;
 for(k=0;k<LM_UP_ROUTINES&&lm_up_entries[k].z!=z;++k){}
 lm_up_input(s,io,hw,(uint16_t)k);
}
/* External entries a run may end at (Z, and the bank for switched ones). */
static const struct { agc_word z; uint8_t bank; } ends[14]={
 {02076u,043u},{05353u,0},{05311u,0},{02334u,010u},{04364u,0},{05105u,0},{05155u,0},{06037u,0},{05504u,0},{07257u,0},
 {03165u,04u},{05314u,0},{05472u,0},{LM_UP_NEWOPS,0}};
int main(void){apollo_overlay_state s;p66_bias_io io;agc_hardware hw;enum exec_status st;unsigned n,i,ended[14]={0};
 check(LM_UP_VECTORS==LM_UP_CASES,"vector count",0);
 /* The C64 rows (chosen by outcome in the freeze) are host cases with their digests. */
 for(n=0;n<LM_UP_C64_COUNT;++n)check(lm_up_c64_case[n]<LM_UP_CASES&&(n==0u||lm_up_c64_case[n-1u]<lm_up_c64_case[n])&&lm_up_c64[n]==lm_up_expected[lm_up_c64_case[n]],"C64 row",n);
 for(n=1;n<LM_UPDATE_OPERAND_COUNT;++n)check(lm_update_operands[n-1u].physical<lm_update_operands[n].physical,"operands sorted",n);
 for(n=1;n<LM_UP_ROUTINES;++n)check(lm_update_routines.entries[n-1u].physical<lm_update_routines.entries[n].physical,"routines sorted",n);
 check(lm_update_interpretive.entries[0].physical<lm_update_interpretive.entries[1].physical,"interpretive sorted",0);
 for(n=0;n<LM_UP_CASES;++n)check(lm_up_digest(&s,(uint16_t)n,&st)==lm_up_expected[n]&&st==EXEC_OK,"oracle",n);
 {agc_routine_entry wrapped[LM_UP_ROUTINES];agc_routine_table tables[2];
  agc_native_program p={0,2,0,0,lm_update_operands,LM_UPDATE_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  agc_routine_entry iwrapped[LM_UP_INTERPRETIVE];agc_routine_table itable;
  for(i=0;i<LM_UP_ROUTINES;++i){wrapped[i].physical=lm_update_routines.entries[i].physical;wrapped[i].call=count;}
  for(i=0;i<LM_UP_INTERPRETIVE;++i){iwrapped[i].physical=lm_update_interpretive.entries[i].physical;iwrapped[i].call=icount;}
  tables[0].entries=wrapped;tables[0].count=LM_UP_ROUTINES;tables[1]=lm_interbank_routines;p.basic=tables;p.hardware=&hw;
  itable.entries=iwrapped;itable.count=LM_UP_INTERPRETIVE;p.interpretive=&itable;p.interpretive_count=1;
  p.interpreter_fetch=(uint16_t)(2048u+LM_UP_NEWOPS-04000u);p.loc=LM_UP_LOC;p.bankset=LM_UP_BANKSET;
  for(n=0;n<LM_UP_CASES;++n){unsigned bank;
   lm_up_input(&s,&io,&hw,(uint16_t)n);
   check(agc_transfer_run(&p,&s,&io,&io.superbank,0xfffeu,24,&steps)==EXEC_UNMAPPED,"coverage run",n);
   bank=s.words[4]>>10;if(bank>=030u&&(io.superbank&0100u))bank+=010u;
   for(i=0;i<14u;++i)if(s.words[5]==ends[i].z&&(!ends[i].bank||bank==ends[i].bank))break;
   check(i<14u,"end point",n);if(i<14u)++ended[i];}
  for(i=0;i<LM_UP_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<LM_UP_INTERPRETIVE;++i)check(ireached[i]>0u,"interpretive entry reached",i);
  for(i=0;i<14u;++i)check(ended[i]>0u,"end reached",i);}
 /* Refusals, each entered once directly. */
 at(&s,&io,&hw,03621u);s.words[LM_UP_UPTEMP]=03543u;check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"TC UPTEMP word",0);
 at(&s,&io,&hw,03627u);s.words[LM_UP_UPTEMP]=03544u;check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"INDEX UPTEMP; TC 1 word",0);
 at(&s,&io,&hw,03412u);s.words[LM_UP_UPVERB]=4u;check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"UPVERB with no branch",0);
 at(&s,&io,&hw,03561u);s.words[LM_UP_UPVERB]=1u;s.words[LM_UP_COMPNUMB]=3u;s.words[LM_UP_UPBUFF+1u]=040u; /* EB 0, S 40: TIME2 */
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"update into a register or counter",0);
 at(&s,&io,&hw,03401u);check(once(&s,0,&hw)==EXEC_OUT_OF_SCOPE,"no io",0);
 at(&s,&io,&hw,03714u);check(once(&s,&io,0)==EXEC_OUT_OF_SCOPE,"UPACTOFF without hardware",0);
 lm_up_input(&s,&io,&hw,(uint16_t)LM_UP_ROUTINES);s.words[023]=1u;check(lm_update_interpretive.entries[0].call(0,&s,&io)==EXEC_OUT_OF_SCOPE,"interpretive word with EDOP not +0",0);
 lm_up_input(&s,&io,&hw,(uint16_t)LM_UP_ROUTINES);check(lm_update_interpretive.entries[0].call(0,&s,0)==EXEC_OUT_OF_SCOPE,"interpretive word without io",0);
 printf("lm-update: %u cases, %u entries, %u interpretive, ends",LM_UP_CASES,LM_UP_ROUTINES,LM_UP_INTERPRETIVE);for(i=0;i<14u;++i)printf(" %u",ended[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
