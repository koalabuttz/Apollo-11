/* SPDX-License-Identifier: MIT */
/* lm-t6rupt: Luminary099's T6RUPT vector, DOT6RUPT, T6JOBCHK/T6JOB, the
 * WRITEP/WRITEU/WRITEV jet writers and the RESUME/NOQRSM/NOQBRSM interrupt
 * exits as native routines, against digests frozen from the original
 * program; every entry and every path is reached; inputs no original
 * producer supplies are refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_t6rupt_cases.h"
#include "lm_t6rupt_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static const agc_word entries[12]={LM_T6RUPT_VECTOR,LM_DOT6RUPT,LM_T6JOBCHK,LM_WRITEP-1u,LM_WRITEP,LM_WRITEU-1u,LM_WRITEU,LM_WRITEV-1u,LM_WRITEV,LM_RESUME,LM_NOQRSM,LM_NOQBRSM};
/* T6JOBCHK paths: return (TIME6 counting), CCSHOLE (+0), T6JOB leaving the
 * clock stopped, T6JOB restarting it, T6JOB restarting it with the next time
 * dropped; and the jets fired: P, U, V. */
enum {P_RETURN,P_CCSHOLE,P_STOPPED,P_RESTARTED,P_DROPPED,P_JET_P,P_JET_U,P_JET_V,PATHS};
static unsigned reached[12],path[PATHS];
#define E6(a) (03000u+((a)-01400u))
static agc_hardware hw;
static enum exec_status run_from(apollo_overlay_state *s,p66_bias_io *io){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&lm_t6rupt_routines;program.basic_count=1;program.hardware=&hw;
 call=agc_transfer_resolve(&program,s,io->superbank);
 return call?call(&program,s,io):EXEC_UNMAPPED;
}
static void refused(apollo_overlay_state *s,p66_bias_io *io,const char *what){static apollo_overlay_state before;p66_bias_io io0=*io;agc_hardware hw0=hw;before=*s;
 check(run_from(s,io)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0)&&!memcmp(&hw,&hw0,sizeof hw0),what,0);
}
int main(void){static apollo_overlay_state s,in;p66_bias_io io;agc_hardware h0;unsigned n,i,kind[3]={0,0,0};
 for(n=0;n<LM_T6_CASES;++n){unsigned k;enum lm_t6_cohort c=lm_t6_cohort((uint16_t)n);
  lm_t6_input(&in,&io,&h0,(uint16_t)n);
  for(i=0;i<12u;++i)if(in.words[5]==entries[i])++reached[i];
  k=lm_t6_run(&s,&io,&hw,(uint16_t)n);
  check(lm_t6_digest(&s,(uint16_t)n)==lm_t6_expected[n],"digest",n);
  /* Only the rope-reader RESUME cases (every fourth) may refuse: BRUPT not the word at ZRUPT-1. */
  check(k==LM_T6_RETURNED||(c==LM_T6_RESUME&&(n-lm_t6_first(c))%4u==3u&&k==LM_T6_REFUSED),"outcome",n);if(k<3u)++kind[k];
  if(c==LM_T6_DOT6RUPT||c==LM_T6_T6JOBCHK){agc_word t=in.words[LM_TIME6];
   if(t==0u)++path[P_CCSHOLE];else if(t!=077777u)++path[P_RETURN];
   else{agc_word x=in.words[E6(LM_NXT6ADR)];++path[x==0u?P_JET_P:x==4u?P_JET_U:P_JET_V];
    if(!(hw.channel13&040000u))++path[P_STOPPED];else if(s.words[E6(LM_T6NEXT)]==037777u&&in.words[E6(LM_T6FURTHA)]!=037777u)++path[P_DROPPED];else ++path[P_RESTARTED];}}
 }
 for(i=0;i<12u;++i)check(reached[i]>0u,"entry reached",i);
 for(i=0;i<PATHS;++i)check(path[i]>0u,"path reached",i);
 /* Inputs no producer supplies. */
 lm_t6_input(&s,&io,&hw,lm_t6_first(LM_T6_DOT6RUPT));s.words[LM_TIME6]=040000u;refused(&s,&io,"DOT6RUPT with TIME6 negative");
 lm_t6_input(&s,&io,&hw,lm_t6_first(LM_T6_DOT6RUPT));s.words[LM_TIME6]=077777u;s.words[E6(LM_NXT6ADR)]=5u;refused(&s,&io,"DOT6RUPT with NXT6ADR not a jet writer");
 lm_t6_input(&s,&io,&hw,lm_t6_first(LM_T6_DOT6RUPT));hw.in_interrupt=0;refused(&s,&io,"DOT6RUPT outside the interrupt");
 lm_t6_input(&s,&io,&hw,lm_t6_first(LM_T6_WRITE));s.words[2]=0100u;refused(&s,&io,"WRITEP-1 returning to erasable");
 lm_t6_input(&s,&io,&hw,lm_t6_first(LM_T6_RESUME));s.words[015]=0;refused(&s,&io,"RESUME with no interrupted address");
 printf("lm-t6rupt: %u cases (%u returned, %u refused); entries",LM_T6_CASES,kind[0],kind[2]);
 for(i=0;i<12u;++i)printf(" %u",reached[i]);
 printf("; return %u, CCSHOLE %u, clock stopped %u, restarted %u, next dropped %u; jets P %u, U %u, V %u; %u checks, %u failures\n",
  path[0],path[1],path[2],path[3],path[4],path[5],path[6],path[7],checks,failures);
 return failures!=0u;
}
