/* SPDX-License-Identifier: MIT */
/* lm-s3435c: Luminary099 S34/35.2's integration head and LOMAT tail and
 * S34/35.5 as routines of the control-transfer ABI (with lm-s3435b's,
 * lm-s3435's and lm-p34's routines and the native INTER-BANK routines),
 * against digests frozen from the original program
 * (tools/lm_s3435c_differential.py). Every case stops at a point outside
 * the routines, and entries outside the routines' contracts are refused
 * with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_s3435c_cases.h"
#include "lm_s3435c_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
/* The routine registered for entry r (interpretive by LOC, basic by Z). */
static agc_routine routine(unsigned r,const apollo_overlay_state *s){
 const agc_routine_table *t=r<10u?&lm_s3435c_interpretive_routines:&lm_s3435c_basic_routines;
 agc_word at=r<10u?s->words[0164]:s->words[5];uint16_t i;
 for(i=0;i<t->count;++i)if(t->entries[i].physical==LM_S3435C_B35(at))return t->entries[i].call;
 return 0;
}
/* The entry's routine called once on an edited entry image: must refuse
 * with the image and device record unchanged. */
static void refuse(apollo_overlay_state *s,p66_bias_io *io,unsigned r,const char *what){
 apollo_overlay_state before=*s;p66_bias_io io0=*io;agc_routine f=routine(r,s);enum exec_status st=f?f(0,s,io):EXEC_OK;
 check(f&&st==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0),what,r);
}
int main(void){apollo_overlay_state s;p66_bias_io io;unsigned n,r,guards=0,stopped[LM_S3435C_ROUTINES]={0};
 check(sorted(&lm_s3435c_interpretive_routines)&&lm_s3435c_interpretive_routines.count==LM_S3435C_INTERPRETIVE_ROUTINES,"interpretive table",0);
 check(sorted(&lm_s3435c_basic_routines)&&lm_s3435c_basic_routines.count==LM_S3435C_BASIC_ROUTINES,"basic table",0);
 check(LM_S3435C_VECTORS==LM_S3435C_CASES,"vector count",LM_S3435C_VECTORS);
 for(n=0;n<LM_S3435C_CASES;++n){
  check(lm_s3435c_digest(&s,n)==lm_s3435c_expected[n],"digest",n);
  if(lm_s3435c_run(&s,&io,n)==LM_S3435C_STOPPED&&lm_s3435c_stop(s.words[5],(agc_word)(s.words[4]&076000u),s.words[2],s.words[0164]))++stopped[n%LM_S3435C_ROUTINES];
 }
 for(r=0;r<LM_S3435C_ROUTINES;++r)check(stopped[r]==LM_S3435C_PER_ROUTINE,"stopped outside the routines",r);
 /* Entries outside the contract, from the first state of each entry. */
 for(r=0;r<LM_S3435C_ROUTINES;++r){
  lm_s3435c_input(&s,&io,r);check(routine(r,&s)!=0,"registered",r);
  lm_s3435c_input(&s,&io,r);s.words[3]|=1u;refuse(&s,&io,r,"EB not a bank");++guards;
  lm_s3435c_input(&s,&io,r);s.words[6]^=1u;refuse(&s,&io,r,"BB incoherent");++guards;
  lm_s3435c_input(&s,&io,r);s.words[4]=070000u;s.words[6]=(agc_word)(070000u|(s.words[3]>>8));refuse(&s,&io,r,"FB not 35");++guards;
  lm_s3435c_input(&s,&io,r);s.words[7]=1u;refuse(&s,&io,r,"ZERO");++guards;
  lm_s3435c_input(&s,&io,r);s.words[0400]|=0100000u;refuse(&s,&io,r,"16-bit word");++guards;
  lm_s3435c_input(&s,&io,r);io.superbank=1u;refuse(&s,&io,r,"SUPERBNK bits");++guards;
  lm_s3435c_input(&s,&io,r);io.interrupt_enabled=2u;refuse(&s,&io,r,"IRQ");++guards;
  if(r<10u){
   lm_s3435c_input(&s,&io,r);s.words[0120]=0400u;refuse(&s,&io,r,"VAC");++guards;
   lm_s3435c_input(&s,&io,r);s.words[0163]=2u;refuse(&s,&io,r,"MODE");++guards;
   lm_s3435c_input(&s,&io,r);s.words[0115]=0u;refuse(&s,&io,r,"INTBIT15");++guards;
   lm_s3435c_input(&s,&io,r);s.words[023]=1u;refuse(&s,&io,r,"EDOP");++guards;
   lm_s3435c_input(&s,&io,r);s.words[067]=1u;refuse(&s,&io,r,"NEWJOB");++guards;
   lm_s3435c_input(&s,&io,r);s.words[0165]^=1u;refuse(&s,&io,r,"BANKSET");++guards;
   lm_s3435c_input(&s,&io,r);s.words[0166]=(agc_word)(s.words[0120]+041u);refuse(&s,&io,r,"PUSHLOC");++guards;
  }
 }
 printf("lm-s3435c: %u cases, %u admission guards; stops",LM_S3435C_CASES,guards);
 for(r=0;r<LM_S3435C_ROUTINES;++r)printf(" %u",stopped[r]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
