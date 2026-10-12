/* SPDX-License-Identifier: MIT */
/* lm-tjet-law: Luminary099 TJET_LAW as a native routine, against digests
 * frozen from the original program; every reachable branch is taken;
 * inputs no original producer supplies are refused with the image
 * unchanged. Built with LM_TJ_PATH counting branches. */
#include <stdio.h>
#include <string.h>
#include "lm_tjet_law_cases.h"
#include "lm_tjet_law_vectors.h"
#define PATHS 60u
unsigned lm_tj_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static void refused(apollo_overlay_state *s,const char *what){apollo_overlay_state before=*s,at=*s;agc_routine call;
 at.words[4]=LM_TJ_FB;at.words[3]=LM_TJ_EB;call=agc_transfer_resolve(&program,&at,0);
 check(call&&call(&program,s,0)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before),what,0);
}
int main(void){apollo_overlay_state s;lm_tj_case c;unsigned n,i,returned=0;uint16_t steps;
 /* Branches no admitted input reaches: NEGEDOT's TS ADRSDIF2 of a
  * one-word index (no overflow); ERRTEST's CCS E at -0 (yaAGC's MP BIT13 of
  * the -0 low word that a -0 E leaves gives +0); CCS ROTSENSE at +0 or -0
  * in ZONE5 and SMALRATE (ROTSENSE is +-SENSOR) and CCS EDOT negative in
  * ZONE3 (EDOT is made positive or -0 before SCALEDOT): the TC CCSHOLE
  * exits and their -0 neighbours. */
 static const unsigned unreachable[]={4u,13u,29u,31u,41u,49u,51u};
 program.basic=&lm_tjet_law_routines;program.basic_count=1;
 for(n=0;n<LM_TJ_CASES;++n){
  check(lm_tj_digest(&s,(uint16_t)n)==lm_tj_expected[n],"digest",n);
  if(!lm_tj_run(&s,(uint16_t)n,&steps))++returned;}
 check(returned==LM_TJ_CASES,"all cases reach the caller's return",returned);
 for(i=0;i<PATHS;++i){unsigned j,skip=0;for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==i;
  if(!skip)check(lm_tj_paths[i]>0u,"branch taken",i);}
 /* Inputs no producer supplies. */
 lm_tj_input(&s,&c,0);s.words[3]=03400u;s.words[6]=(agc_word)(LM_TJ_FB|7u);refused(&s,"TJETLAW under EBANK7");
 lm_tj_input(&s,&c,1);s.words[03000u+LM_TJ_AXISCTR-01400u]=2u;refused(&s,"AXISCTR +2");
 lm_tj_input(&s,&c,2);s.words[03000u+LM_TJ_AXISCTR-01400u]=077777u;refused(&s,"AXISCTR -0");
 lm_tj_input(&s,&c,3);s.words[2]=01000u;refused(&s,"return into erasable");
 lm_tj_input(&s,&c,4);s.words[2]=050000u;refused(&s,"return not a TC");
 printf("lm-tjet-law: %u cases returned; branches",returned);
 for(i=0;i<PATHS;++i)printf(" %u",lm_tj_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
