/* SPDX-License-Identifier: MIT */
/* lm-throttle: Luminary099 THROTTLE_CONTROL_ROUTINES and FASTCHNG as native
 * routines, against digests frozen from the original program; every
 * routine and every reachable branch is taken; inputs no original producer
 * supplies are refused with the image unchanged. Built with LM_THR_PATH
 * counting branches. */
#include <stdio.h>
#include <string.h>
#include "lm_throttle_cases.h"
#include "lm_throttle_vectors.h"
#define P(z) (uint16_t)(031u*1024u+((z)-02000u))
#define PATHS 25u
unsigned lm_thr_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
#define POINTS 9u
static const uint16_t points[POINTS]={P(02216u),P(02221u),P(02224u),P(02230u),P(02303u),P(02370u),P(02371u),P(02400u),P(03740u)};
static unsigned reached[POINTS];
static void cover(uint16_t n){apollo_overlay_state s;lm_flight_io io;lm_thr_case c;uint16_t point,stop;unsigned i,k;agc_routine call;
 lm_thr_input(&s,&io,&c,n);stop=agc_fixed_physical(c.stop_z,c.stop_fb==0177777u?0u:c.stop_fb,0);
 for(k=0;k<24u;++k){point=agc_transfer_point(&program,&s,0);if(point==stop)return;
  call=agc_transfer_resolve(&program,&s,0);if(!call)return;
  for(i=0;i<POINTS;++i)if(points[i]==point)++reached[i];
  if(call(&program,&s,&io)!=EXEC_OK)return;}
}
static void refused(apollo_overlay_state *s,lm_flight_io *io,const char *what){apollo_overlay_state before=*s,at=*s;agc_routine call;
 at.words[4]=LM_THR_FB;at.words[3]=03400u;call=agc_transfer_resolve(&program,&at,0);
 check(call&&call(&program,s,io)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before),what,0);
}
int main(void){apollo_overlay_state s;lm_flight_io io;lm_thr_case c;unsigned n,i,returned=0;uint16_t steps;
 /* Branches no admitted input reaches: TS THRUST after a TS PSEUDO55 of
  * the same unoverflowed A; TS FC and TS FCOLD of a 15-bit word; TS
  * PHSNAME3 of a fixed return; WHATOUT's TS PIFPSET (A not overflowed). */
 static const unsigned unreachable[]={9u,21u,22u,23u,24u};
 program.basic=&lm_throttle_routines;program.basic_count=1;
 for(n=0;n<LM_THR_CASES;++n){
  check(lm_thr_digest(&s,(uint16_t)n)==lm_thr_expected[n],"digest",n);
  if(!lm_thr_run(&s,&io,(uint16_t)n,&steps))++returned;}
 check(returned==LM_THR_CASES,"all cases reach the caller's return",returned);
 memset(lm_thr_paths,0,sizeof lm_thr_paths);
 for(n=0;n<LM_THR_CASES;++n)cover((uint16_t)n);
 for(i=0;i<POINTS;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<PATHS;++i){unsigned j,skip=0;for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==i;
  if(!skip)check(lm_thr_paths[i]>0u,"branch taken",i);}
 /* Inputs no producer supplies. */
 lm_thr_input(&s,&io,&c,0);s.words[3]=01400u;s.words[6]=(agc_word)(LM_THR_FB|1u);refused(&s,&io,"THROTTLE under EBANK1");
 lm_thr_input(&s,&io,&c,2);s.words[2]=01000u;refused(&s,&io,"FLATOUT return into erasable");
 lm_thr_input(&s,&io,&c,3);s.words[2]=050000u;refused(&s,&io,"WHATOUT return not a TC");
 lm_thr_input(&s,&io,&c,0);s.words[5]=LM_THR_MASSMULT;s.words[2]=0177u;refused(&s,&io,"MASSMULT return into erasable");
 lm_thr_input(&s,&io,&c,0);s.words[5]=LM_THR_FASTCHNG;s.words[2]=0100000u|02303u;refused(&s,&io,"FASTCHNG Q with bit 16");
 lm_thr_input(&s,&io,&c,0);s.words[5]=02303u;s.words[01607u-01400u+03400u]=0;refused(&s,&io,"RTNHOLD zero");
 printf("lm-throttle: %u cases returned; routines",returned);
 for(i=0;i<POINTS;++i)printf(" %u",reached[i]);
 printf("; branches");for(i=0;i<PATHS;++i)printf(" %u",lm_thr_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
