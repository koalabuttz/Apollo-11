/* SPDX-License-Identifier: MIT */
/* lm-fresh-start: Luminary099 FRESH_START_AND_RESTART bank 05 and GOTOPOOH
 * cases through the trampoline, against digests frozen from the original
 * program; every registered entry is reached and each run ends at an entry
 * outside the file (DUMMYJOB +2, GOP00FIX, RESTARTS, MMDSPLAY, ALARM) or a
 * KLEAN caller's return (SWRETURN or another fixed-fixed word); an ERESTORE
 * repair into the registers or counters, a TC Q to no return point, a
 * missing io record and a fresh start without the hardware record (channel
 * 11) are refused. */
#include <stdio.h>
#include "lm_fresh_start_cases.h"
#include "lm_fresh_start_vectors.h"
#include "lm_interbank_routines.h"
#include "../src/lm_fresh_start_operands.inc"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[LM_FS_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((p66_bias_io *)io)->superbank);
 for(i=0;i<LM_FS_ROUTINES;++i)if(lm_fresh_start_routines.entries[i].physical==at){++reached[i];return lm_fresh_start_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,lm_fresh_io *io,agc_hardware *hw){
 agc_native_program p={&lm_fresh_start_routines,1,0,0,lm_fresh_operands,LM_FRESH_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.hardware=hw;call=agc_transfer_resolve(&p,s,io?io->control.superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The first case entering at z. */
static void at(apollo_overlay_state *s,lm_fresh_io *io,agc_hardware *hw,agc_word z){unsigned k;
 for(k=0;k<LM_FS_ROUTINES&&lm_fresh_entries[k]!=z;++k){}
 lm_fs_input(s,io,hw,(uint16_t)k);
}
int main(void){apollo_overlay_state s;lm_fresh_io io;agc_hardware hw;enum exec_status st;unsigned n,i,ended[7]={0,0,0,0,0,0,0};
 check(LM_FS_VECTORS==LM_FS_CASES,"vector count",0);
 for(n=1;n<LM_FRESH_OPERAND_COUNT;++n)check(lm_fresh_operands[n-1u].physical<lm_fresh_operands[n].physical,"operands sorted",n);
 for(n=1;n<LM_FS_ROUTINES;++n)check(lm_fresh_start_routines.entries[n-1u].physical<lm_fresh_start_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<LM_FS_CASES;++n)check(lm_fs_digest(&s,(uint16_t)n,&st)==lm_fs_expected[n]&&st==EXEC_OK,"oracle",n);
 {agc_routine_entry wrapped[LM_FS_ROUTINES];agc_routine_table tables[2];
  agc_native_program p={0,2,0,0,lm_fresh_operands,LM_FRESH_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<LM_FS_ROUTINES;++i){wrapped[i].physical=lm_fresh_start_routines.entries[i].physical;wrapped[i].call=count;}
  tables[0].entries=wrapped;tables[0].count=LM_FS_ROUTINES;tables[1]=lm_interbank_routines;p.basic=tables;p.hardware=&hw;
  for(n=0;n<LM_FS_CASES;++n){unsigned bank;agc_word z;
   lm_fs_input(&s,&io,&hw,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.control.superbank,LM_FS_SWRETURN_POINT,24,&steps);
   check(st==EXEC_UNMAPPED||st==EXEC_OK,"coverage run",n);
   z=s.words[5];bank=s.words[4]>>10;if(bank>=030u&&(io.control.superbank&0100u))bank+=010u;
   if(st==EXEC_OK){check(z==04631u&&s.words[2]==3u,"SWRETURN after a KLEAN entry",n);++ended[5];}
   else if(z==03210u&&bank==01u)++ended[0];          /* DUMMYJOB +2 */
   else if(z==02024u&&bank==04u)++ended[1];          /* GOP00FIX */
   else if(z==03526u&&bank==01u)++ended[2];          /* RESTARTS, through SWCALL */
   else if(z==05315u)++ended[3];                     /* MMDSPLAY */
   else if(z==05567u)++ended[4];                     /* ALARM */
   else if(z>=07000u&&z<=07777u&&s.words[2]==3u)++ended[6];  /* a KLEAN caller's return */
   else check(0,"end point",n);}
  for(i=0;i<LM_FS_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<7u;++i)check(ended[i]>0u,"end reached",i);}
 /* Refusals, each entered once directly. */
 at(&s,&io,&hw,02667u);io.channel16=0;s.words[LM_FS_ERESTORE]=s.words[LM_FS_SKEEP7]=040u; /* GOPROG: repair TIME3 */
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"ERESTORE repair into a counter",0);
 at(&s,&io,&hw,02643u);s.words[2]=02500u;check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"TC Q to no return point",0);
 at(&s,&io,&hw,02447u);check(once(&s,0,&hw)==EXEC_OUT_OF_SCOPE,"no io",0);
 at(&s,&io,&hw,02447u);check(once(&s,&io,0)==EXEC_OUT_OF_SCOPE,"fresh start without hardware",0);
 printf("lm-fresh-start: %u cases, %u entries, ends",LM_FS_CASES,LM_FS_ROUTINES);for(i=0;i<7u;++i)printf(" %u",ended[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
