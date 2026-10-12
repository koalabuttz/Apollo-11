/* SPDX-License-Identifier: MIT */
/* lm-alarm: Luminary099 ALARM_AND_ABORT cases through the trampoline, against
 * digests frozen from the original program; every registered entry is
 * reached; each run ends at a point outside the file (a caller's return,
 * PRIODSPR, ENEMA, V37KLEAN, MR.KLEAN, SERVIDLE, or SWRETURN after FLAGS); a Q at no compiled site,
 * an ALMCADR word that is no return address, a negative alarm code that
 * overflows MULTFAIL's TS, a missing io record and WHIMPER without the hardware record are refused. */
#include <stdio.h>
#include "lm_alarm_cases.h"
#include "lm_alarm_vectors.h"
#include "lm_interbank_routines.h"
#define LM_ALARM_SITE_LIST
#include "../src/lm_alarm_sites.inc"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[LM_AL_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((p66_bias_io *)io)->superbank);
 for(i=0;i<LM_AL_ROUTINES;++i)if(lm_alarm_routines.entries[i].physical==at){++reached[i];return lm_alarm_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw){
 agc_native_program p={&lm_alarm_routines,1,0,0,lm_alarm_operands,LM_ALARM_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.hardware=hw;call=agc_transfer_resolve(&p,s,io?io->superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The input of the first case of kind k. */
static void kind(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,unsigned k){lm_al_input(s,io,hw,(uint16_t)k);}
int main(void){apollo_overlay_state s;p66_bias_io io;agc_hardware hw;enum exec_status st;unsigned n,i,ends[7]={0,0,0,0,0,0,0};
 check(LM_AL_VECTORS==LM_AL_CASES,"vector count",0);
 for(n=1;n<LM_ALARM_OPERAND_COUNT;++n)check(lm_alarm_operands[n-1u].physical<lm_alarm_operands[n].physical,"operands sorted",n);
 for(n=0;n<LM_ALARM_SITE_COUNT;++n){agc_word w=0;
  check(agc_site_lookup(lm_alarm_operands,LM_ALARM_OPERAND_COUNT,lm_alarm_site_list[n].physical,&w)&&w>0u&&w<040000u,"site code",n);}
 for(n=1;n<LM_AL_ROUTINES;++n)check(lm_alarm_routines.entries[n-1u].physical<lm_alarm_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<LM_AL_CASES;++n)check(lm_al_digest(&s,(uint16_t)n,&st)==lm_al_expected[n]&&st==EXEC_OK,"oracle",n);
 /* Coverage over the same cases, with a counting wrapper; where each ends. */
 {agc_routine_entry wrapped[LM_AL_ROUTINES];agc_routine_table tables[2];
  agc_native_program p={0,2,0,0,lm_alarm_operands,LM_ALARM_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<LM_AL_ROUTINES;++i){wrapped[i].physical=lm_alarm_routines.entries[i].physical;wrapped[i].call=count;}
  tables[0].entries=wrapped;tables[0].count=LM_AL_ROUTINES;tables[1]=lm_interbank_routines;p.basic=tables;p.hardware=&hw;
  for(n=0;n<LM_AL_CASES;++n){uint16_t z,fb;
   lm_al_input(&s,&io,&hw,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.superbank,LM_AL_SWRETURN_POINT,24,&steps);
   check(st==EXEC_UNMAPPED||st==EXEC_OK,"coverage run",n);
   z=s.words[5];fb=(uint16_t)(s.words[4]>>10);
   if(st==EXEC_OK){check(z==04631u&&s.words[2]==3u,"SWRETURN after FLAGS",n);++ends[6];}
   else if(z==LM_AL_PRIODSPR&&fb==010u)++ends[1];
   else if(z==02765u&&fb==05u)++ends[2];             /* ENEMA */
   else if((z==02652u||z==02643u)&&fb==05u)++ends[3]; /* V37KLEAN, MR.KLEAN */
   else if(z==03321u&&fb==027u)++ends[4];             /* SERVIDLE */
   else if(z>=02000u&&z<=07777u&&!(z>=04616u&&z<=05752u))++ends[0];
   else{++ends[5];check(0,"end point",n);}}
  for(i=0;i<LM_AL_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<7u;++i)if(i!=5u)check(ends[i]>0u,"end kind reached",i);}
 /* Refusals, each from the first case of a kind, entered once directly. */
 kind(&s,&io,&hw,0);s.words[2]=(agc_word)(s.words[2]+1u);           /* POODOO1 site: Q one past the code */
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"Q at no compiled site",0);
 kind(&s,&io,&hw,lm_al_kinds()-2u);s.words[LM_AL_ALMCADR]=040000u;    /* ALMCADR */
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"ALMCADR word",0);
 kind(&s,&io,&hw,LM_ALARM_SITE_COUNT+2u);s.words[0]=0160000u;         /* VARALARM with a negative code */
 s.words[0375]=s.words[0376]=s.words[0377]=1u;
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"negative code overflows MULTFAIL",0);
 kind(&s,&io,&hw,0);check(once(&s,0,&hw)==EXEC_OUT_OF_SCOPE,"no io",0);
 kind(&s,&io,&hw,LM_ALARM_SITE_COUNT+3u);                             /* WHIMPER */
 check(s.words[5]==LM_AL_WHIMPER&&once(&s,&io,0)==EXEC_OUT_OF_SCOPE,"WHIMPER without hardware",0);
 printf("lm-alarm: %u cases, %u sites, ends %u caller/%u PRIODSPR/%u ENEMA/%u KLEAN/%u SERVIDLE/%u SWRETURN, %u checks, %u failures\n",
        LM_AL_CASES,LM_ALARM_SITE_COUNT,ends[0],ends[1],ends[2],ends[3],ends[4],ends[6],checks,failures);
 return failures!=0u;
}
