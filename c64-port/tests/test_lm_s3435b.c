/* SPDX-License-Identifier: MIT */
/* lm-s3435b: Luminary099 S34/35.3, GOINT and INTINT as interpretive
 * routines of the control-transfer ABI (with lm-s3435's LOMAT and NOVRWRT),
 * against digests frozen from the original program
 * (tools/lm_s3435bb_differential.py). Every case
 * stops at a NEWOPS outside the routines, and entries outside the routines'
 * contracts are refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_s3435b_cases.h"
#include "lm_s3435b_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
/* One trampoline run of an edited entry image: must refuse unchanged. */
static void refuse(apollo_overlay_state *s,p66_bias_io *io,const char *what,unsigned n){
 agc_routine_table interp[2];agc_native_program program={0,0,0,0,0,0,LM_S3435B_FETCH,0164,0165,0,0,0};
 apollo_overlay_state before=*s;p66_bias_io io0=*io;uint16_t steps;enum exec_status st;
 interp[0]=lm_s3435b_routines;interp[1]=lm_s3435_routines;program.interpretive=interp;program.interpretive_count=2;
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,4,&steps);
 check(st==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0),what,n);
}
int main(void){apollo_overlay_state s;p66_bias_io io;unsigned n,guards=0,stopped[LM_S3435B_ROUTINES]={0};
 check(sorted(&lm_s3435b_routines)&&lm_s3435b_routines.count==LM_S3435B_ROUTINES,"routine table",0);
 check(LM_S3435B_VECTORS==LM_S3435B_CASES,"vector count",LM_S3435B_VECTORS);
 for(n=0;n<LM_S3435B_CASES;++n){
  check(lm_s3435b_digest(&s,n)==lm_s3435b_expected[n],"digest",n);
  if(lm_s3435b_run(&s,&io,n)==LM_S3435B_STOPPED&&s.words[5]==06070u&&!lm_s3435b_owned_newops(s.words[4],s.words[0164]))++stopped[n%LM_S3435B_ROUTINES];
 }
 for(n=0;n<LM_S3435B_ROUTINES;++n)check(stopped[n]==LM_S3435B_PER_ROUTINE,"stopped at a NEWOPS outside",n);
 /* Entries outside the contract, from the first state of each routine. */
 for(n=0;n<LM_S3435B_ROUTINES;++n){
  lm_s3435b_input(&s,&io,n);s.words[3]|=1u;refuse(&s,&io,"EB not a bank",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[6]^=1u;refuse(&s,&io,"BB incoherent",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[7]=1u;refuse(&s,&io,"ZERO",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[0400]|=0100000u;refuse(&s,&io,"16-bit word",n);++guards;
  lm_s3435b_input(&s,&io,n);io.superbank=1u;refuse(&s,&io,"SUPERBNK bits",n);++guards;
  lm_s3435b_input(&s,&io,n);io.interrupt_enabled=2u;refuse(&s,&io,"IRQ",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[0120]=0400u;refuse(&s,&io,"VAC",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[0163]=2u;refuse(&s,&io,"MODE",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[0115]=0u;refuse(&s,&io,"INTBIT15",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[023]=1u;refuse(&s,&io,"EDOP",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[067]=1u;refuse(&s,&io,"NEWJOB",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[0165]^=1u;refuse(&s,&io,"BANKSET",n);++guards;
  lm_s3435b_input(&s,&io,n);s.words[0166]=(agc_word)(s.words[0120]+051u);refuse(&s,&io,"PUSHLOC",n);++guards;
 }
 printf("lm-s3435b: %u cases, %u admission guards; stops [%u %u %u %u %u %u %u]; %u checks, %u failures\n",LM_S3435B_CASES,guards,stopped[0],stopped[1],stopped[2],stopped[3],stopped[4],stopped[5],stopped[6],checks,failures);
 return failures!=0u;
}
