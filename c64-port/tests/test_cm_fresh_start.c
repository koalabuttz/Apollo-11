/* SPDX-License-Identifier: MIT */
/* cm-fresh-start: Comanche055 FRESH_START_AND_RESTART bank 05 and GOTOPOOH
 * cases through the trampoline, against digests frozen from the original
 * program; every registered entry is reached and each run ends at an entry
 * outside the file (DUMMYJOB +2, RESTARTS, MMDSPLAY, WAITLIST, PHASCHNG,
 * VAC5STOR, GOP00FIX, NOQRSM +1, ALARM, STOPRATE, SETCOARS) or a KLEAN
 * caller's return (SWRETURN or another fixed-fixed word); an ERESTORE
 * repair into the registers or counters, a TC Q to no return point, a
 * missing io record and a channel this file does not use are refused. */
#include <stdio.h>
#include "cm_fresh_start_cases.h"
#include "cm_fresh_start_vectors.h"
#include "cm_interbank_routines.h"
#include "../src/cm_fresh_start_operands.inc"
#define ENDS 13u
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[CM_FS_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((cm_native_io *)io)->superbank);
 for(i=0;i<CM_FS_ROUTINES;++i)if(cm_fresh_start_routines.entries[i].physical==at){++reached[i];return cm_fresh_start_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,cm_fresh_io *io){
 agc_native_program p={&cm_fresh_start_routines,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.operands=cm_interbank_site_table(&p.operand_count);call=agc_transfer_resolve(&p,s,io?io->control.superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The first case entering at z. */
static void at(apollo_overlay_state *s,cm_fresh_io *io,agc_word z){unsigned k;
 for(k=0;k<CM_FS_ROUTINES&&cm_fresh_entries[k]!=z;++k){}
 cm_fs_input(s,io,(uint16_t)k);
}
int main(void){apollo_overlay_state s;cm_fresh_io io;enum exec_status st;unsigned n,i,ended[ENDS];
 for(i=0;i<ENDS;++i)ended[i]=0;
 check(CM_FS_VECTORS==CM_FS_CASES,"vector count",0);
 for(n=1;n<CM_FRESH_OPERAND_COUNT;++n)check(cm_fresh_operands[n-1u].physical<cm_fresh_operands[n].physical,"operands sorted",n);
 for(n=1;n<CM_FS_ROUTINES;++n)check(cm_fresh_start_routines.entries[n-1u].physical<cm_fresh_start_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<CM_FS_CASES;++n)check(cm_fs_digest(&s,(uint16_t)n,&st)==cm_fresh_start_expected[n]&&st==EXEC_OK,"oracle",n);
 {agc_routine_entry wrapped[CM_FS_ROUTINES];agc_routine_table tables[2];
  agc_native_program p={0,2,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<CM_FS_ROUTINES;++i){wrapped[i].physical=cm_fresh_start_routines.entries[i].physical;wrapped[i].call=count;}
  tables[0].entries=wrapped;tables[0].count=CM_FS_ROUTINES;tables[1]=cm_interbank_routines;p.basic=tables;
  p.operands=cm_interbank_site_table(&p.operand_count);
  for(n=0;n<CM_FS_CASES;++n){unsigned bank;agc_word z;
   cm_fs_input(&s,&io,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.control.superbank,CM_FS_SWRETURN_POINT,24,&steps);
   check(st==EXEC_UNMAPPED||st==EXEC_OK,"coverage run",n);
   z=s.words[5];bank=s.words[4]>>10;if(bank>=030u&&(io.control.superbank&0100u))bank+=010u;
   if(st==EXEC_OK){check(z==04675u&&s.words[2]==3u,"SWRETURN after a KLEAN entry",n);++ended[0];}
   else if(z==03225u&&bank==01u)++ended[1];          /* DUMMYJOB +2 */
   else if(z==03520u&&bank==01u)++ended[2];          /* RESTARTS, through SWCALL */
   else if(z==05360u)++ended[3];                     /* MMDSPLAY */
   else if(z==05245u)++ended[4];                     /* WAITLIST */
   else if(z==05412u)++ended[5];                     /* PHASCHNG */
   else if(z==02000u&&bank==020u)++ended[6];         /* VAC5STOR, through BANKCALL */
   else if(z==02000u&&bank==04u)++ended[7];          /* GOP00FIX */
   else if(z==05336u)++ended[8];                     /* NOQRSM +1 */
   else if(z==05650u)++ended[9];                     /* ALARM */
   else if(z==04526u)++ended[10];                    /* STOPRATE */
   else if(z==03027u&&bank==07u)++ended[11];         /* SETCOARS, through IBNKCALL */
   else if(z>=07000u&&z<=07777u&&s.words[2]==3u)++ended[12];  /* a KLEAN caller's return */
   else check(0,"end point",n);}
  for(i=0;i<CM_FS_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<ENDS;++i)check(ended[i]>0u,"end reached",i);}
 /* Refusals, each entered once directly. */
 at(&s,&io,02526u);s.words[CM_FS_ERESTORE]=s.words[CM_FS_SKEEP7]=040u;io.channel33=0;io.control.navigation_keys=0;
 s.words[5]=02535u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"ERESTORE repair into a counter",0);   /* after VAC5STOR: repair TIME3 */
 at(&s,&io,02501u);s.words[2]=02500u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"TC Q to no return point",0);
 at(&s,&io,02342u);check(once(&s,0)==EXEC_OUT_OF_SCOPE,"no io",0);
 at(&s,&io,02342u);io.control.superbank=0200u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"SUPERBNK outside bits 5-7",0);
 printf("cm-fresh-start: %u cases, %u entries, ends",CM_FS_CASES,CM_FS_ROUTINES);for(i=0;i<ENDS;++i)printf(" %u",ended[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
