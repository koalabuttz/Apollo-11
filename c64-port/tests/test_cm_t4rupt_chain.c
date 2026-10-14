/* SPDX-License-Identifier: MIT */
/* cm-t4rupt-chain: whole T4RUPT interrupts of Comanche055 run through the
 * trampoline over the three routine tables of T4RUPT_PROGRAM.agc, against
 * digests frozen from the original program; every run leaves the file at a
 * continuation the original reaches, and each exit and both chain lengths
 * (the core alone, the core then a monitor) are reached. */
#include <stdio.h>
#include "cm_t4rupt_chain_cases.h"
#include "cm_t4rupt_chain_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* RESUME, NOVAC (PROCKEY), IBNKCALL (OPTTEST's OPTDRIVE, SETCOARS), ALARM, VARALARM, WAITLIST. */
static const agc_word exits[6]={05333u,05134u,04740u,05650u,05764u,05245u};
static unsigned reached[6],chain[CCH_MAX_STEPS+1u];
int main(void){static apollo_overlay_state s;unsigned n,i,kind[5]={0,0,0,0,0},longest=0;
 for(n=0;n<CCH_CASES;++n){static cch_io io;unsigned k;uint16_t steps;
  check(cch_digest(&s,(uint16_t)n)==cch_expected[n],"digest",n);
  k=cch_run(&s,&io,(uint16_t)n,&steps);check(k==CCH_RETURNED,"outcome",n);if(k<5u)++kind[k];
  for(i=0;i<6u;++i)if(s.words[5]==exits[i])break;
  check(i<6u,"exit",n);if(i<6u)++reached[i];
  if(steps<=CCH_MAX_STEPS)++chain[steps];
  if(steps>longest)longest=steps;}
 for(i=0;i<6u;++i)check(reached[i]>0u,"exit reached",i);
 check(chain[1]>0u&&chain[2]>0u,"chains of one routine (core) and two (core, then a monitor)",0);
 printf("cm-t4rupt-chain: %u interrupts (%u returned, %u refused, %u with both channel copies changed), up to %u routines each (1: %u, 2: %u); to RESUME %u, NOVAC %u, IBNKCALL %u, ALARM %u, VARALARM %u, WAITLIST %u; %u checks, %u failures\n",
  CCH_CASES,kind[0],kind[2],kind[4],longest,chain[1],chain[2],reached[0],reached[1],reached[2],reached[3],reached[4],reached[5],checks,failures);
 return failures!=0u;
}
