/* SPDX-License-Identifier: MIT */
/* lm-t4rupt-rr: the rendezvous radar monitor and GPMATRIX of Luminary099
 * T4RUPT_PROGRAM.agc as native routines, against digests frozen from the
 * original program; every entry returns a continuation the original leaves,
 * and every external service is reached; an SP trig return with an
 * overflow in A is refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_t4rupt_rr_cases.h"
#include "lm_t4rupt_rr_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* SPSIN, SPCOS, SETTRKF, WAITLIST, RRLIMCHK, RESUME, ALARM. */
static const agc_word external[7]={05033u,05032u,04564u,05203u,04523u,05270u,05567u};
static unsigned reached[7];
static uint16_t first(agc_word z){uint16_t n;for(n=0;n<LRR_CASES;++n)if(lrr_entry(n)==z)return n;return 0;}
int main(void){static apollo_overlay_state s;unsigned n,i,kind[3]={0,0,0};
 for(n=0;n<LRR_CASES;++n){static lrr_io io;unsigned k;agc_word z;
  check(lrr_digest(&s,(uint16_t)n)==lrr_expected[n],"digest",n);
  k=lrr_run(&s,&io,(uint16_t)n);check(k==LRR_RETURNED,"outcome",n);if(k<3u)++kind[k];
  z=s.words[5];for(i=0;i<7u;++i)if(z==external[i])break;
  check(i<7u,"continuation",n);if(i<7u)++reached[i];}
 for(i=0;i<7u;++i)check(reached[i]>0u,"continuation reached",i);
 /* SPSIN/SPCOS returns with an overflow in A: refused, nothing changed. */
 {static const agc_word ret[4]={03134u,03137u,03142u,03151u};
  for(i=0;i<4u;++i){static apollo_overlay_state before;lrr_io io,io0;agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
   lrr_input(&s,&io,first(ret[i]));s.words[0]=i%2u?0100000u:040000u;before=s;io0=io;
   program.basic=&lm_t4rupt_rr_routines;program.basic_count=1;program.hardware=&io.hw;
   call=agc_transfer_resolve(&program,&s,io.lm.superbank);
   check(call&&call(&program,&s,&io.lm)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io),"overflow in A refused",i);}}
 printf("lm-t4rupt-rr: %u cases (%u returned, %u refused); to SPSIN %u, SPCOS %u, SETTRKF %u, WAITLIST %u, RRLIMCHK %u, RESUME %u, ALARM %u; %u checks, %u failures\n",
  LRR_CASES,kind[0],kind[2],reached[0],reached[1],reached[2],reached[3],reached[4],reached[5],reached[6],checks,failures);
 return failures!=0u;
}
