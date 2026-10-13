/* SPDX-License-Identifier: MIT */
/* cm-alarm: Comanche055 ALARM_AND_ABORT cases through the trampoline, against
 * digests frozen from the original program; every registered entry is
 * reached; each run ends at a point outside the file (a caller's return,
 * PRIODSPR, ENEMA, MR.KLEAN, VAC5STOR or DOWNFLAG); a Q or ALMCADR at no
 * compiled site or in erasable, an ALMCADR word that is no return address, a negative alarm
 * code that overflows MULTFAIL's TS, a missing io record and WHIMPER without
 * the hardware record are refused. */
#include <stdio.h>
#include "cm_alarm_cases.h"
#include "cm_alarm_vectors.h"
#include "cm_interbank_routines.h"
#define CM_ALARM_SITE_LIST
#include "../src/cm_alarm_operands.inc"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[CM_AL_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((cm_native_io *)io)->superbank);
 for(i=0;i<CM_AL_ROUTINES;++i)if(cm_alarm_routines.entries[i].physical==at){++reached[i];return cm_alarm_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw){
 agc_native_program p={&cm_alarm_routines,1,0,0,cm_alarm_operands,CM_ALARM_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.hardware=hw;call=agc_transfer_resolve(&p,s,io?io->superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The input of the first case of kind k. */
static void kind(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,unsigned k){cm_al_input(s,io,hw,(uint16_t)k);}
static unsigned other(agc_word z){
 unsigned k;apollo_overlay_state s;cm_native_io io;agc_hardware hw;
 for(k=CM_ALARM_SITE_COUNT;k<cm_al_kinds();++k){kind(&s,&io,&hw,k);if(s.words[5]==z)return k;}
 return 0;
}
int main(void){apollo_overlay_state s;cm_native_io io;agc_hardware hw;enum exec_status st;unsigned n,i,ends[7]={0,0,0,0,0,0,0},first_alarm=0;
 check(CM_AL_VECTORS==CM_AL_CASES,"vector count",0);
 for(n=1;n<CM_ALARM_OPERAND_COUNT;++n)check(cm_alarm_operands[n-1u].physical<cm_alarm_operands[n].physical,"operands sorted",n);
 for(n=0;n<CM_ALARM_SITE_COUNT;++n){agc_word w=0;
  check(agc_site_lookup(cm_alarm_operands,CM_ALARM_OPERAND_COUNT,cm_alarm_site_list[n].physical,&w)&&w>0u&&w<040000u,"site code",n);}
 while(cm_alarm_site_list[first_alarm].callee!=CM_AS_ALARM)++first_alarm;
 for(n=1;n<CM_AL_ROUTINES;++n)check(cm_alarm_routines.entries[n-1u].physical<cm_alarm_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<CM_AL_CASES;++n)check(cm_al_digest(&s,(uint16_t)n,&st)==cm_al_expected[n]&&st==EXEC_OK,"oracle",n);
 /* Coverage over the same cases, with a counting wrapper; where each ends. */
 {agc_routine_entry wrapped[CM_AL_ROUTINES];agc_routine_table tables[2];
  agc_native_program p={0,2,0,0,cm_alarm_operands,CM_ALARM_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<CM_AL_ROUTINES;++i){wrapped[i].physical=cm_alarm_routines.entries[i].physical;wrapped[i].call=count;}
  tables[0].entries=wrapped;tables[0].count=CM_AL_ROUTINES;tables[1]=cm_interbank_routines;p.basic=tables;p.hardware=&hw;
  for(n=0;n<CM_AL_CASES;++n){uint16_t z,fb;
   cm_al_input(&s,&io,&hw,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.superbank,AGC_TRANSFER_NONE,24,&steps);
   check(st==EXEC_UNMAPPED,"coverage run",n);
   z=s.words[5];fb=(uint16_t)(s.words[4]>>10);
   if(z==CM_AL_PRIODSPR&&fb==010u)++ends[1];
   else if(z==02651u&&fb==05u)++ends[2];              /* ENEMA */
   else if(z==02501u&&fb==05u)++ends[3];              /* MR.KLEAN */
   else if(z==02000u&&fb==020u)++ends[4];             /* VAC5STOR */
   else if(z==CM_AL_DOWNFLAG)++ends[5];
   else if(z>=02000u&&z<=07777u&&!(z>=04662u&&z<=04775u)&&!(z>=05650u&&z<=05772u)&&!(z>=07755u&&z<=07774u))++ends[0];
   else{++ends[6];check(0,"end point",n);}}
  for(i=0;i<CM_AL_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<6u;++i)check(ends[i]>0u,"end kind reached",i);}
 /* Refusals, each from the first case of a kind, entered once directly. */
 kind(&s,&io,&hw,first_alarm);s.words[2]=(agc_word)(s.words[2]+1u);     /* ALARM site: Q one past the code */
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"Q at no compiled site",0);
 kind(&s,&io,&hw,other(CM_AL_BAILOUT_RET));s.words[CM_AL_ALMCADR]=(agc_word)(s.words[CM_AL_ALMCADR]+1u);
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"ALMCADR at no compiled site",0);
 kind(&s,&io,&hw,other(CM_AL_BAILOUT_RET));s.words[CM_AL_ALMCADR]=01000u;
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"ALMCADR in erasable",0);
 kind(&s,&io,&hw,first_alarm);s.words[2]=01000u;
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"Q in erasable",0);
 kind(&s,&io,&hw,other(CM_AL_ALMCADR));s.words[CM_AL_ALMCADR]=040000u;
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"ALMCADR word",0);
 kind(&s,&io,&hw,other(CM_AL_VARALARM));s.words[0]=0160000u;           /* VARALARM with a negative code */
 s.words[0375]=s.words[0376]=s.words[0377]=1u;
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"negative code overflows MULTFAIL",0);
 kind(&s,&io,&hw,first_alarm);check(once(&s,0,&hw)==EXEC_OUT_OF_SCOPE,"no io",0);
 kind(&s,&io,&hw,other(CM_AL_WHIMPER_M1));
 check(s.words[5]==CM_AL_WHIMPER_M1&&once(&s,&io,0)==EXEC_OUT_OF_SCOPE,"WHIMPER without hardware",0);
 printf("cm-alarm: %u cases, %u sites, ends %u caller/%u PRIODSPR/%u ENEMA/%u MR.KLEAN/%u VAC5STOR/%u DOWNFLAG, %u checks, %u failures\n",
        CM_AL_CASES,CM_ALARM_SITE_COUNT,ends[0],ends[1],ends[2],ends[3],ends[4],ends[5],checks,failures);
 return failures!=0u;
}
