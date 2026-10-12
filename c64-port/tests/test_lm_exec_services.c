/* SPDX-License-Identifier: MIT */
/* lm-exec-services: Luminary099 NOVAC, FINDVAC, WAITLIST, TWIDDLE and the
 * LONGCALL chain as native routines, against digests frozen from the original
 * program; every routine and every service alarm is reached; inputs no
 * original producer supplies are refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_exec_services_cases.h"
#include "lm_exec_services_vectors.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static agc_routine_table tables[2];
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0};
static const uint16_t points[10]={FF(LM_EXEC_NOVAC),FF(LM_EXEC_FINDVAC),FF(LM_EXEC_TWIDDLE),FF(LM_EXEC_WAITLIST),FF(LM_EXEC_LONGCALL),
 B1(LM_EXEC_LNGCALL2),B1(LM_EXEC_LONGCYCL),B1(LM_EXEC_LONGRTRN),B1(LM_EXEC_MUCHRTRN),B1(LM_EXEC_GETCADR)};
static unsigned reached[10],alarms[5];
/* The trampoline loop, recording each lm_exec routine entered and the alarm
 * a case ends in (1201, 1202, 1203, 1204 from WAITLIST, 1204 from LONGCALL). */
static void cover(uint16_t n){apollo_overlay_state s;p66_bias_io io;lm_exs_case c;uint16_t point,stop,last=0;unsigned i,k;agc_routine call;
 lm_exs_input(&s,&io,&c,n);stop=agc_fixed_physical(c.stop_z,c.stop_fb==0177777u?0u:c.stop_fb,c.superbank);
 for(k=0;k<48u;++k){point=agc_transfer_point(&program,&s,c.superbank);if(point==stop)return;
  call=agc_transfer_resolve(&program,&s,c.superbank);if(!call)break;
  for(i=0;i<10u;++i)if(points[i]==point){++reached[i];last=point;}
  if(call(&program,&s,&io)!=EXEC_OK)return;}
 if(s.words[5]==LM_EXEC_BAILOUT1)++alarms[s.words[2]==02617u?0:s.words[2]==02703u?1:2];
 else if(s.words[5]==LM_EXEC_POODOO1)++alarms[last==B1(LM_EXEC_LNGCALL2)?4:3];
}
static enum exec_status run_from(apollo_overlay_state *s,uint16_t stop){p66_bias_io io={0,0};agc_word sb=0;uint16_t steps;
 return agc_transfer_run(&program,s,&io,&sb,stop,48,&steps);
}
/* A refusal leaves the image as it was. */
static void refused(apollo_overlay_state *s,const char *what){apollo_overlay_state before=*s;
 check(run_from(s,0xfffeu)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before),what,0);
}
int main(void){apollo_overlay_state s;p66_bias_io io;lm_exs_case c;unsigned n,i,kind[3]={0,0,0},dpbit14=0;uint16_t steps;
 tables[0]=lm_exec_routines;tables[1]=lm_restart_routines;program.basic=tables;program.basic_count=2;
 program.operands=lm_exec_site_table(&program.operand_count);
 check(program.operand_count==268u,"268 operand words",program.operand_count);
 for(n=0;n<LM_EXS_CASES;++n){unsigned k=lm_exs_run(&s,(uint16_t)n,&steps);
  check(lm_exs_digest(&s,(uint16_t)n)==lm_exs_expected[n],"digest",n);
  /* Only LONGCYCL reaching DPBIT14 is refused. */
  check(k<3u&&(k!=LM_EXS_REFUSED||n>=LM_EXS_TASK_FIRST),"outcome",n);if(k<3u)++kind[k];
  if(k==LM_EXS_REFUSED&&!dpbit14)dpbit14=n;
  cover((uint16_t)n);}
 for(i=0;i<10u;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<5u;++i)check(alarms[i]>0u,"alarm reached",i);
 /* Inputs no producer supplies. */
 lm_exs_input(&s,&io,&c,LM_EXS_SITE_FIRST);s.words[2]=(agc_word)(s.words[2]+2u);refused(&s,"fixed Q off a call site");
 lm_exs_input(&s,&io,&c,LM_EXS_SITE_FIRST);s.words[2]=07776u;refused(&s,"Q above 07775");
 lm_exs_input(&s,&io,&c,LM_EXS_TASK_FIRST+1u);s.words[6]=02002u;s.words[3]=01000u;refused(&s,"LONGCYCL bank");
 lm_exs_input(&s,&io,&c,LM_EXS_TASK_FIRST+1u);s.words[5]=LM_EXEC_LNGCALL2;s.words[6]=02002u;s.words[3]=01000u;refused(&s,"LNGCALL2 bank");
 lm_exs_input(&s,&io,&c,LM_EXS_TASK_FIRST+1u);s.words[5]=LM_EXEC_LONGRTRN;s.words[LM_EXEC_LONGEXIT]=0;refused(&s,"LONGEXIT 0");
 lm_exs_input(&s,&io,&c,LM_EXS_TASK_FIRST);s.words[LM_EXEC_LONGCADR]=010000u;refused(&s,"LONGCADR above 07777");
 check(dpbit14!=0u,"a DPBIT14 case",0);
 lm_exs_input(&s,&io,&c,(uint16_t)dpbit14);refused(&s,"LONGCYCL DPBIT14");
 /* The dummy 1.2SPOT (DEC 100, 2CADR TASKOVER) second part: FINDVAC with priority 0144. */
 lm_exs_input(&s,&io,&c,0);s.words[5]=03526u;s.words[0161]=0;s.words[0154]=2;
 check(run_from(&s,FF(LM_RS_SWRETURN))==EXEC_OUT_OF_SCOPE&&s.words[5]==LM_EXEC_FINDVAC&&s.words[0]==0144u&&s.words[0705]==LM_EXEC_TASKOVER,"dummy 1.2SPOT",0);
 printf("lm-exec-services: %u cases (%u returned, %u alarm, %u refused); routines",LM_EXS_CASES,kind[0],kind[1],kind[2]);
 for(i=0;i<10u;++i)printf(" %u",reached[i]);
 printf("; alarms 1201 %u 1202 %u 1203 %u 1204 %u LONGPOOH %u; %u checks, %u failures\n",alarms[0],alarms[1],alarms[2],alarms[3],alarms[4],checks,failures);
 return failures!=0u;
}
