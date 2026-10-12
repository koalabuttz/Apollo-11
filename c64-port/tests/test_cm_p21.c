/* SPDX-License-Identifier: MIT */
/* cm-p21: Comanche055 P21 (ground tracking) as routines of the control-transfer ABI,
 * against digests frozen from the original program
 * (tools/cm_p21_differential.py). Every case stops at a point outside the
 * routines, and entries outside the routines' contracts are refused with the
 * image unchanged. */
#include <stdio.h>
#include <string.h>
#include "cm_p21_cases.h"
#include "cm_p21_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
/* The routine registered for entry r (interpretive by LOC, basic by Z). */
static agc_routine routine(unsigned r,const apollo_overlay_state *s){
 const agc_routine_table *t=r<CM_P21_INTERPRETIVE?&cm_p21_interpretive_routines:&cm_p21_basic_routines;
 agc_word at=r<CM_P21_INTERPRETIVE?s->words[0164]:s->words[5];uint16_t i;
 for(i=0;i<t->count;++i)if(t->entries[i].physical==CM_P21_B37(at))return t->entries[i].call;
 return 0;
}
/* The entry's routine called once on an edited entry image: must refuse
 * with the image and device record unchanged. */
static void refuse(apollo_overlay_state *s,p66_bias_io *io,unsigned r,const char *what){
 apollo_overlay_state before=*s;p66_bias_io io0=*io;agc_routine f=routine(r,s);enum exec_status st=f?f(0,s,io):EXEC_OK;
 check(f&&st==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0),what,r);
}
int main(void){apollo_overlay_state s;p66_bias_io io;unsigned n,r,guards=0,stopped[CM_P21_ROUTINES]={0};
 check(sorted(&cm_p21_interpretive_routines)&&cm_p21_interpretive_routines.count==CM_P21_INTERPRETIVE_ROUTINES,"interpretive table",0);
 check(sorted(&cm_p21_basic_routines)&&cm_p21_basic_routines.count==CM_P21_BASIC_ROUTINES,"basic table",0);
 check(CM_P21_VECTORS==CM_P21_CASES,"vector count",CM_P21_VECTORS);
 for(n=0;n<CM_P21_CASES;++n){
  check(cm_p21_digest(&s,n)==cm_p21_expected[n],"digest",n);
  if(cm_p21_run(&s,&io,n)==CM_P21_STOPPED&&cm_p21_stop(s.words[5],(agc_word)(s.words[4]&076000u),s.words[2],s.words[0164]))++stopped[n%CM_P21_ROUTINES];
 }
 for(r=0;r<CM_P21_ROUTINES;++r)check(stopped[r]==CM_P21_PER_ROUTINE,"stopped outside the routines",r);
 /* Entries outside the contract, from the first state of each entry. */
 for(r=0;r<CM_P21_ROUTINES;++r){
  cm_p21_input(&s,&io,r);check(routine(r,&s)!=0,"registered",r);
  cm_p21_input(&s,&io,r);s.words[3]|=1u;refuse(&s,&io,r,"EB not a bank");++guards;
  cm_p21_input(&s,&io,r);s.words[6]^=1u;refuse(&s,&io,r,"BB incoherent");++guards;
  cm_p21_input(&s,&io,r);s.words[4]=072000u;s.words[6]=(agc_word)(072000u|(s.words[3]>>8));refuse(&s,&io,r,"FB not 37");++guards;
  cm_p21_input(&s,&io,r);s.words[7]=1u;refuse(&s,&io,r,"ZERO");++guards;
  cm_p21_input(&s,&io,r);s.words[0400]|=0100000u;refuse(&s,&io,r,"16-bit word");++guards;
  cm_p21_input(&s,&io,r);io.superbank=1u;refuse(&s,&io,r,"SUPERBNK bits");++guards;
  cm_p21_input(&s,&io,r);io.interrupt_enabled=2u;refuse(&s,&io,r,"IRQ");++guards;
  if(r<CM_P21_INTERPRETIVE){
   cm_p21_input(&s,&io,r);s.words[0120]=0400u;refuse(&s,&io,r,"VAC");++guards;
   cm_p21_input(&s,&io,r);s.words[0163]=2u;refuse(&s,&io,r,"MODE");++guards;
   cm_p21_input(&s,&io,r);s.words[0115]=0u;refuse(&s,&io,r,"INTBIT15");++guards;
   cm_p21_input(&s,&io,r);s.words[023]=1u;refuse(&s,&io,r,"EDOP");++guards;
   cm_p21_input(&s,&io,r);s.words[067]=1u;refuse(&s,&io,r,"NEWJOB");++guards;
   cm_p21_input(&s,&io,r);s.words[0165]^=1u;refuse(&s,&io,r,"BANKSET");++guards;
   cm_p21_input(&s,&io,r);s.words[0166]=(agc_word)(s.words[0120]+041u);refuse(&s,&io,r,"PUSHLOC");++guards;
  }
 }
 printf("cm-p21: %u cases, %u admission guards; stops",CM_P21_CASES,guards);
 for(r=0;r<CM_P21_ROUTINES;++r)printf(" %u",stopped[r]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
