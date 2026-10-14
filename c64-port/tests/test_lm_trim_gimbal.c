/* SPDX-License-Identifier: MIT */
/* lm-trim-gimbal: Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM as native
 * routines, against digests frozen from the original program; every
 * routine and every reachable branch is taken; inputs no original producer
 * supplies are refused with the image unchanged. Built with LM_TG_PATH
 * counting branches. */
#include <stdio.h>
#include <string.h>
#include "lm_trim_gimbal_cases.h"
#include "lm_trim_gimbal_vectors.h"
#include "lm_interbank_routines.h"
#define P16(z) (uint16_t)(016u*1024u+((z)-02000u))
#define P21(z) (uint16_t)(021u*1024u+((z)-02000u))
#define PATHS 87u
unsigned lm_tg_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
#define POINTS 4u
static const uint16_t points[POINTS]={P21(LM_TG_GTS),P16(LM_TG_ACDTC12),P21(LM_TG_TIMEGMBL),P21(LM_TG_TIMERET)};
static unsigned reached[POINTS];
/* Counts the routines of this file each case passes through. */
static void cover(uint16_t n){apollo_overlay_state s;lm_flight_io io;lm_tg_case c;uint16_t point;unsigned i,k;agc_routine call;
 lm_tg_input(&s,&io,&c,n);
 for(k=0;k<8u;++k){point=agc_transfer_point(&program,&s,0);
  call=agc_transfer_resolve(&program,&s,0);if(!call)return;
  for(i=0;i<POINTS;++i)if(points[i]==point)++reached[i];
  if((program.hardware=&io.hw,call(&program,&s,&io))!=EXEC_OK)return;}
}
static void refused(apollo_overlay_state *s,lm_flight_io *io,const char *what){apollo_overlay_state before=*s,at=*s;lm_flight_io bio=*io;agc_routine call;
 at.words[3]=LM_TG_EB;at.words[4]=s->words[5]==LM_TG_ACDTC12?LM_TG_FB16:LM_TG_FB21;call=agc_transfer_resolve(&program,&at,0);
 check(call&&(program.hardware=&io->hw,call(&program,s,io))==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&bio,sizeof bio),what,0);
}
int main(void){apollo_overlay_state s,in;lm_flight_io io,iin;lm_tg_case c;unsigned n,i,stopped=0,refusals=0;uint16_t steps;agc_routine call;
 /* Branches no admitted input reaches:
  *  6, 63, 83, 84: the -0 exits of CCS QRCNTR (GTSQAXIS -3 and LOUPE) and
  *   the negative and -0 exits of CCS QRNDXER: QRCNTR is 2, 0 or, after
  *   TS SAVESR overflows, +-1; QRNDXER is 2 or 0;
  *  23, 30: the -0 exits of CCS DEL: DEL is +1, +0 or -1;
  *  41, 42: the negative and -0 exits of CCS A on SCALDONE's MASK BIT13;
  *  17: CCS FUNCTION +1's +0 exit, whose TCF +2 reaches TS DEL with A = +0
  *   as the -0 exit (19, taken) does: FUNCTION is the DAS of K2CNTRAL and
  *   A2CNTRAL, words of one sign each, and never ends (+-0, +0) (none in
  *   3,000,000 random GTS passes);
  *  43: ROOTCYCL's TS SCRATCH overflow: the first Newton step, from
  *   ROOTHALF (HALFARG below 1/4) or POSMAX (HALFARG below 1/2), stays
  *   below 1, and later steps only decrease. */
 static const unsigned unreachable[]={6u,17u,23u,30u,41u,42u,43u,63u,83u,84u};
 program.basic=&lm_trim_gimbal_routines;program.basic_count=1;
 for(n=0;n<LM_TG_CASES;++n){unsigned kind;
  check(lm_tg_digest(&s,(uint16_t)n)==lm_tg_expected[n],"digest",n);
  kind=lm_tg_run(&s,&io,(uint16_t)n,&steps);
  if(kind<LM_TG_REFUSED)++stopped;
  if(kind==LM_TG_REFUSED){++refusals;lm_tg_input(&in,&iin,&c,(uint16_t)n);
   check(!memcmp(&s,&in,sizeof s)&&io.hw.channel12==iin.hw.channel12,"refused GTS leaves the image unchanged",n);}
  check(kind!=LM_TG_NONE,"every case stops or is refused",n);}
 check(stopped+refusals==LM_TG_CASES,"all cases reach their stop",stopped);
 memset(lm_tg_paths,0,sizeof lm_tg_paths);
 for(n=0;n<LM_TG_CASES;++n)cover((uint16_t)n);
 for(i=0;i<POINTS;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<PATHS;++i){unsigned j,skip=0;for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==i;
  if(!skip)check(lm_tg_paths[i]>0u,"branch taken",i);}
 /* The case program's site operand is the compiled table's own. */
 {uint16_t ni,k;const agc_site_operand *ib=lm_interbank_site_table(&ni);int found=0;
  for(k=0;k<ni;++k)found|=ib[k].physical==lm_tg_sites[0].physical&&ib[k].word==lm_tg_sites[0].word;
  check(found,"site operand from the compiled table",0);}
 /* Inputs no producer supplies. */
 lm_tg_input(&s,&io,&c,0);s.words[3]=01400u;s.words[6]=(agc_word)(LM_TG_FB21|1u);refused(&s,&io,"GTS under EBANK1");
 lm_tg_input(&s,&io,&c,2);s.words[3]=03400u;s.words[6]=(agc_word)(LM_TG_FB21|7u);refused(&s,&io,"TIMEGMBL under EBANK7");
 lm_tg_input(&s,&io,&c,1);s.words[2]=01000u;refused(&s,&io,"ACDT+C12 return into erasable");
 lm_tg_input(&s,&io,&c,1);s.words[2]=050000u;refused(&s,&io,"ACDT+C12 return not a TC");
 lm_tg_input(&s,&io,&c,3);s.words[071]=01000u;refused(&s,&io,"TIMEGMBL return into erasable");
 lm_tg_input(&s,&io,&c,3);s.words[071]=030000u;refused(&s,&io,"TIMEGMBL return not a TC");
 lm_tg_input(&s,&io,&c,0);call=agc_transfer_resolve(&program,&s,0);
 {apollo_overlay_state before=s;check(call&&call(&program,&s,0)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),"GTS without io",0);}
 printf("lm-trim-gimbal: %u cases stopped, %u GTS refusals; routines",stopped,refusals);
 for(i=0;i<POINTS;++i)printf(" %u",reached[i]);
 printf("; branches");for(i=0;i<PATHS;++i)printf(" %u",lm_tg_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
