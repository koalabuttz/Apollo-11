/* SPDX-License-Identifier: MIT */
/* lm-dapidler: Luminary099 DAPIDLER_PROGRAM as native routines, against
 * digests frozen from the original program; every routine and every
 * reachable branch is taken; inputs no original producer supplies are
 * refused with the image unchanged. Built with LM_DI_PATH counting
 * branches. */
#include <stdio.h>
#include <string.h>
#include "lm_dapidler_cases.h"
#include "lm_dapidler_vectors.h"
#include "lm_exec_routines.h"
#include "lm_interbank_routines.h"
#define P16(z) (uint16_t)(016u*1024u+((z)-02000u))
#define PATHS 38u
unsigned lm_di_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
#define POINTS 9u
static const uint16_t points[POINTS]={P16(02000u),P16(02016u),P16(02024u),P16(02041u),P16(02042u),P16(02050u),P16(02161u),
 P16(02163u),(uint16_t)(020u*1024u+(02226u-02000u))};
static unsigned reached[POINTS];
/* Counts the routines of this file each case passes through. */
static void cover(uint16_t n){apollo_overlay_state s;lm_flight_io io;lm_di_case c;uint16_t point;unsigned i,k;agc_routine call;
 lm_di_input(&s,&io,&c,n);
 for(k=0;k<32u;++k){point=agc_transfer_point(&program,&s,0);
  call=agc_transfer_resolve(&program,&s,0);if(!call)return;
  for(i=0;i<POINTS;++i)if(points[i]==point)++reached[i];
  if(call(&program,&s,&io)!=EXEC_OK)return;}
}
static void refused(apollo_overlay_state *s,lm_flight_io *io,const char *what){apollo_overlay_state before=*s,at=*s;lm_flight_io bio=*io;agc_routine call;
 at.words[3]=LM_DI_EB;at.words[4]=s->words[5]>=02226u?LM_DI_FB20:LM_DI_FB16;call=agc_transfer_resolve(&program,&at,0);
 check(call&&call(&program,s,io)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&bio,sizeof bio),what,0);
}
int main(void){apollo_overlay_state s;lm_flight_io io;lm_di_case c;unsigned n,i,stopped=0;uint16_t steps;agc_routine call;
 /* Branches no admitted input reaches: the negative and -0 exits of every
  * CCS on a MASK or RAND result (CHEKBITS, DAPIDLER, CHEKMORE, ALTDSPLY,
  * NEEDLES3), which is never negative, and CCS DINDX's (DINDX is 2, 1 or
  * 0). */
 static const unsigned unreachable[]={3u,4u,7u,8u,11u,12u,18u,19u,22u,23u,28u,29u,36u,37u};
 program.basic=&lm_dapidler_routines;program.basic_count=1;
 for(n=0;n<LM_DI_CASES;++n){
  check(lm_di_digest(&s,(uint16_t)n)==lm_di_expected[n],"digest",n);
  if(lm_di_run(&s,&io,(uint16_t)n,&steps)<LM_DI_NONE)++stopped;}
 check(stopped==LM_DI_CASES,"all cases reach their stop",stopped);
 memset(lm_di_paths,0,sizeof lm_di_paths);
 for(n=0;n<LM_DI_CASES;++n)cover((uint16_t)n);
 for(i=0;i<POINTS;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<PATHS;++i){unsigned j,skip=0;for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==i;
  if(!skip)check(lm_di_paths[i]>0u,"branch taken",i);}
 /* The case program's site operands are the compiled tables' own. */
 {uint16_t ne,ni,k;const agc_site_operand *ex=lm_exec_site_table(&ne),*ib=lm_interbank_site_table(&ni);
  for(i=0;i<LM_DI_SITES;++i){int found=0;
   for(k=0;k<ne;++k)found|=ex[k].physical==lm_di_sites[i].physical&&ex[k].word==lm_di_sites[i].word;
   for(k=0;k<ni;++k)found|=ib[k].physical==lm_di_sites[i].physical&&ib[k].word==lm_di_sites[i].word;
   check(found,"site operand from the compiled tables",i);}}
 /* Inputs no producer supplies. */
 lm_di_input(&s,&io,&c,2);s.words[3]=01400u;s.words[6]=(agc_word)(LM_DI_FB16|1u);refused(&s,&io,"DAPIDLER under EBANK1");
 lm_di_input(&s,&io,&c,8);s.words[3]=03400u;s.words[6]=(agc_word)(LM_DI_FB20|7u);refused(&s,&io,"ALTDSPLY under EBANK7");
 lm_di_input(&s,&io,&c,0);s.words[2]=01000u;refused(&s,&io,"CHEKBITS return into erasable");
 lm_di_input(&s,&io,&c,1);s.words[2]=050000u;refused(&s,&io,"CHEKMORE return not a TC");
 lm_di_input(&s,&io,&c,3);call=agc_transfer_resolve(&program,&s,0);
 {apollo_overlay_state before=s;check(call&&call(&program,&s,0)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),"CHECKUP without io",0);}
 printf("lm-dapidler: %u cases stopped; routines",stopped);
 for(i=0;i<POINTS;++i)printf(" %u",reached[i]);
 printf("; branches");for(i=0;i<PATHS;++i)printf(" %u",lm_di_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
