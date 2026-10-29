/* SPDX-License-Identifier: MIT */
/* lm-landing-displays: Luminary099 LANDING_ANALOG_DISPLAYS as native
 * routines, against digests frozen from the original program; every
 * routine and every reachable branch is taken; inputs no original producer
 * supplies are refused with the image unchanged. Built with LM_LAD_PATH
 * counting branches. */
#include <stdio.h>
#include <string.h>
#include "lm_landing_displays_cases.h"
#include "lm_landing_displays_vectors.h"
#define PATHS 115u
unsigned lm_lad_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static unsigned reached[LM_LAD_ENTRIES];
static void refused(apollo_overlay_state *s,lm_flight_io *io,const char *what){apollo_overlay_state before=*s;lm_flight_io bio=*io;agc_routine call;
 call=agc_transfer_resolve(&program,s,0);
 check(call&&(program.hardware=&io->hw,call(&program,s,io))==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&bio,sizeof bio),what,0);
}
int main(void){apollo_overlay_state s,in;lm_flight_io io,iin;unsigned n,i,left=0,refusals=0;uint16_t steps;agc_routine call;
 /* Branches no admitted input reaches:
  *  1, 2, 5, 6, 37, 38, 47, 49, 107, 108: the negative and -0 exits of a
  *   CCS A after MASK of a positive constant (SWANDBIT, BIT7, R10FLBIT,
  *   BIT8) or RAND of BIT6, which is never negative;
  *  14, 42, 109, 111: the overflow skips of TS IMODES33 and TS FLAGWRD1
  *   after a MASK, whose result never overflows;
  *  18: TS ALTBITS after CCS ALTBITS of -0 stores +0, which does not skip;
  *  27, 28, 29, 34, 35: CCS ALTSAVE +1 and CCS ALTSAVE negative and -0 (and
  *   the TS between): OLDDATA leaves both words non-negative (POSMAX + 1
  *   plus ALTSAVE +1, POSMAX plus the carry plus ALTSAVE, each kept only on
  *   positive overflow) and ZERODATA stores +0, +0;
  *  57: CCS LATVEL -0: LATVEL and LATVEL +1 are DDOUBL of VELCONV (9422)
  *   times a word, whose upper half is +-0 only for a factor of +-0, and
  *   MP by +-0 gives +0;
  *  113, 114: CCS ITEMP5 -0 and negative: VMONITOR stores 1, then 0. */
 static const unsigned unreachable[]={1u,2u,5u,6u,14u,18u,27u,28u,29u,34u,35u,37u,38u,42u,47u,49u,57u,107u,108u,109u,111u,113u,114u};
 program.basic=&lm_landing_displays_routines;program.basic_count=1;
 memset(lm_lad_paths,0,sizeof lm_lad_paths);
 for(n=0;n<LM_LAD_CASES;++n){unsigned kind;
  check(lm_lad_digest(&s,(uint16_t)n)==lm_lad_expected[n],"digest",n);
  kind=lm_lad_run(&s,&io,(uint16_t)n,&steps);
  if(kind==LM_LAD_LEFT){++left;++reached[n%LM_LAD_ENTRIES];}
  if(kind==LM_LAD_REFUSED){++refusals;lm_lad_input(&in,&iin,(uint16_t)n);
   check(!memcmp(&s,&in,sizeof s)&&!memcmp(&io,&iin,sizeof io),"refused case leaves the image unchanged",n);}
  check(kind!=LM_LAD_NONE,"every case leaves the file or is refused",n);}
 check(left+refusals==LM_LAD_CASES,"all cases reach their stop",left);
 for(i=0;i<LM_LAD_ENTRIES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<PATHS;++i){unsigned j,skip=0;for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==i;
  if(!skip)check(lm_lad_paths[i]>0u,"branch taken",i);}
 /* Inputs no producer supplies. */
 lm_lad_input(&s,&io,0);s.words[3]=03000u;refused(&s,&io,"LANDISP under EBANK6");
 lm_lad_input(&s,&io,1);call=agc_transfer_resolve(&program,&s,0);s.words[4]=s.words[6]=020000u;
 {apollo_overlay_state before=s;lm_flight_io bio=io;
  check(call&&(program.hardware=&io.hw,call(&program,&s,&io))==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&bio,sizeof io),"INTLZE under FB 10",0);}
 lm_lad_input(&s,&io,3);call=agc_transfer_resolve(&program,&s,0);
 {apollo_overlay_state before=s;check(call&&call(&program,&s,0)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),"21,2476 without io",0);}
 printf("lm-landing-displays: %u cases left the file, %u refusals; entries",left,refusals);
 for(i=0;i<LM_LAD_ENTRIES;++i)printf(" %u",reached[i]);
 printf("; branches");for(i=0;i<PATHS;++i)printf(" %u",lm_lad_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
