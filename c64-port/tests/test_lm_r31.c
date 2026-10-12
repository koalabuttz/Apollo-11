/* SPDX-License-Identifier: MIT */
/* lm-r31: Luminary099 R31 (rendezvous display) as routines of the control-transfer ABI,
 * against digests frozen from the original program
 * (tools/lm_r31_differential.py). Every case stops at a point outside the
 * routines, and entries outside the routines' contracts are refused with the
 * image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_r31_cases.h"
#include "lm_r31_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
/* The routine registered for entry r (interpretive by LOC, basic by Z). */
static agc_routine routine(unsigned r,const apollo_overlay_state *s){
 const agc_routine_table *t=r<LM_R31_INTERPRETIVE?&lm_r31_interpretive_routines:&lm_r31_basic_routines;
 agc_word at=r<LM_R31_INTERPRETIVE?s->words[0164]:s->words[5];uint16_t i;
 for(i=0;i<t->count;++i)if(t->entries[i].physical==(r>=LM_R31_ROUTINES-4u?LM_R31_B40(at):LM_R31_B37(at)))return t->entries[i].call;
 return 0;
}
/* The entry's routine called once on an edited entry image: must refuse
 * with the image and device record unchanged. */
static void refuse(apollo_overlay_state *s,p66_bias_io *io,unsigned r,const char *what){
 apollo_overlay_state before=*s;p66_bias_io io0=*io;agc_routine f=routine(r,s);enum exec_status st=f?f(0,s,io):EXEC_OK;
 check(f&&st==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0),what,r);
}
int main(void){apollo_overlay_state s;p66_bias_io io;unsigned n,r,guards=0,stopped[LM_R31_ROUTINES]={0};
 check(sorted(&lm_r31_interpretive_routines)&&lm_r31_interpretive_routines.count==LM_R31_INTERPRETIVE_ROUTINES,"interpretive table",0);
 check(sorted(&lm_r31_basic_routines)&&lm_r31_basic_routines.count==LM_R31_BASIC_ROUTINES,"basic table",0);
 check(LM_R31_VECTORS==LM_R31_CASES,"vector count",LM_R31_VECTORS);
 for(n=0;n<LM_R31_CASES;++n){
  check(lm_r31_digest(&s,n)==lm_r31_expected[n],"digest",n);
  if(lm_r31_run(&s,&io,n)==LM_R31_STOPPED&&lm_r31_stop(s.words[5],(agc_word)(s.words[4]&076000u),s.words[2],s.words[0164]))++stopped[n%LM_R31_ROUTINES];
 }
 for(r=0;r<LM_R31_ROUTINES;++r)check(stopped[r]==LM_R31_PER_ROUTINE,"stopped outside the routines",r);
 /* Entries outside the contract, from the first state of each entry. */
 for(r=0;r<LM_R31_ROUTINES;++r){
  lm_r31_input(&s,&io,r);check(routine(r,&s)!=0,"registered",r);
  lm_r31_input(&s,&io,r);s.words[3]|=1u;refuse(&s,&io,r,"EB not a bank");++guards;
  lm_r31_input(&s,&io,r);s.words[6]^=1u;refuse(&s,&io,r,"BB incoherent");++guards;
  lm_r31_input(&s,&io,r);s.words[4]=072000u;s.words[6]=(agc_word)(072000u|(s.words[3]>>8));refuse(&s,&io,r,"FB not 37 or 40");++guards;
  lm_r31_input(&s,&io,r);s.words[7]=1u;refuse(&s,&io,r,"ZERO");++guards;
  lm_r31_input(&s,&io,r);s.words[0400]|=0100000u;refuse(&s,&io,r,"16-bit word");++guards;
  lm_r31_input(&s,&io,r);io.superbank=1u;refuse(&s,&io,r,"SUPERBNK bits");++guards;
  lm_r31_input(&s,&io,r);io.interrupt_enabled=2u;refuse(&s,&io,r,"IRQ");++guards;
  if(r<LM_R31_INTERPRETIVE){
   lm_r31_input(&s,&io,r);s.words[0120]=0400u;refuse(&s,&io,r,"VAC");++guards;
   lm_r31_input(&s,&io,r);s.words[0163]=2u;refuse(&s,&io,r,"MODE");++guards;
   lm_r31_input(&s,&io,r);s.words[0115]=0u;refuse(&s,&io,r,"INTBIT15");++guards;
   lm_r31_input(&s,&io,r);s.words[023]=1u;refuse(&s,&io,r,"EDOP");++guards;
   lm_r31_input(&s,&io,r);s.words[067]=1u;refuse(&s,&io,r,"NEWJOB");++guards;
   lm_r31_input(&s,&io,r);s.words[0165]^=1u;refuse(&s,&io,r,"BANKSET");++guards;
   lm_r31_input(&s,&io,r);s.words[0166]=(agc_word)(s.words[0120]+041u);refuse(&s,&io,r,"PUSHLOC");++guards;
  }
 }
 printf("lm-r31: %u cases, %u admission guards; stops",LM_R31_CASES,guards);
 for(r=0;r<LM_R31_ROUTINES;++r)printf(" %u",stopped[r]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
