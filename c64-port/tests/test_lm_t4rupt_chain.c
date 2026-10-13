/* SPDX-License-Identifier: MIT */
/* lm-t4rupt-chain: whole T4RUPT interrupts of Luminary099 run through the
 * trampoline over the three routine tables of T4RUPT_PROGRAM.agc, against
 * digests frozen from the original program; every run leaves the file at a
 * continuation the original reaches, and each T4JUMP target and exit is
 * reached. */
#include <stdio.h>
#include "lm_t4rupt_chain_cases.h"
#include "lm_t4rupt_chain_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* RESUME, NOVAC (PROCKEY), RCSMONIT, SPSIN (GPMATRIX), ALARM, VARALARM,
 * WAITLIST, IBNKCALL, SETTRKF, RRLIMCHK. */
static const agc_word exits[10]={05270u,05072u,03156u,05033u,05567u,05744u,05203u,04674u,04564u,04523u};
static unsigned reached[10],chain[LCH_MAX_STEPS+1u];
int main(void){static apollo_overlay_state s;unsigned n,i,kind[3]={0,0,0},longest=0;
 for(n=0;n<LCH_CASES;++n){static lch_io io;unsigned k;uint16_t steps;
  check(lch_digest(&s,(uint16_t)n)==lch_expected[n],"digest",n);
  k=lch_run(&s,&io,(uint16_t)n,&steps);check(k==LCH_RETURNED,"outcome",n);if(k<3u)++kind[k];
  for(i=0;i<10u;++i)if(s.words[5]==exits[i])break;
  check(i<10u,"exit",n);if(i<10u)++reached[i];
  if(steps<=LCH_MAX_STEPS)++chain[steps];
  if(steps>longest)longest=steps;}
 for(i=0;i<10u;++i)check(reached[i]>0u,"exit reached",i);
 check(chain[1]>0u&&chain[2]>0u,"chains of one routine (core) and two (core, then a monitor)",0);
 printf("lm-t4rupt-chain: %u interrupts (%u returned, %u refused), up to %u routines each; to RESUME %u, NOVAC %u, RCSMONIT %u, SPSIN %u, ALARM %u, VARALARM %u, WAITLIST %u, IBNKCALL %u, SETTRKF %u, RRLIMCHK %u; %u checks, %u failures\n",
  LCH_CASES,kind[0],kind[2],longest,reached[0],reached[1],reached[2],reached[3],reached[4],reached[5],reached[6],reached[7],reached[8],reached[9],checks,failures);
 return failures!=0u;
}
