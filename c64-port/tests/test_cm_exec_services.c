/* SPDX-License-Identifier: MIT */
/* cm-exec-services: Comanche055 NOVAC, FINDVAC, WAITLIST, TWIDDLE, the
 * LONGCALL chain, FIXDELAY, VARDELAY, TASKOVER and the T3RUPT vector as
 * native routines, against digests frozen from the original program; every
 * routine and every service alarm is reached; inputs no original producer
 * supplies are refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "cm_exec_services_cases.h"
#include "cm_exec_services_vectors.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))
#define ROUTINES 14u
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static agc_hardware hw; /* the case's CPU state while it runs */
static const uint16_t points[ROUTINES]={FF(CM_EXEC_NOVAC),FF(CM_EXEC_FINDVAC),FF(CM_EXEC_TWIDDLE),FF(CM_EXEC_WAITLIST),FF(CM_EXEC_LONGCALL),
 B1(CM_EXEC_LNGCALL2),B1(CM_EXEC_LONGCYCL),B1(CM_EXEC_LONGRTRN),B1(CM_EXEC_MUCHRTRN),B1(CM_EXEC_GETCADR),
 FF(CM_EXEC_FIXDELAY),FF(CM_EXEC_VARDELAY),FF(CM_EXEC_TASKOVER),FF(CM_EXEC_T3RUPT_VECTOR)};
static unsigned reached[ROUTINES],alarms[4];
/* The trampoline loop, recording each cm_exec routine entered and the alarm
 * a case ends in (1201, 1202 and 1203 at BAILOUT; 1204 at POODOO). A
 * delay's own 1204 has no producer: FIXDELAY and VARDELAY only receive
 * positive delays (cm_waitlist_internal refuses others). */
static void cover(uint16_t n){apollo_overlay_state s;cm_native_io io;cm_exs_case c;uint16_t point;unsigned i,k;agc_routine call;
 cm_exs_input(&s,&io,&c,n);hw=c.hw;
 for(k=0;k<48u;++k){point=agc_transfer_point(&program,&s,io.superbank);
  for(i=0;i<c.stops;++i)if(s.words[5]==c.stop_z[i]&&(c.stop_fb[i]==CM_EXS_ANY_FB||s.words[4]==c.stop_fb[i]))return;
  call=agc_transfer_resolve(&program,&s,io.superbank);if(!call)break;
  for(i=0;i<ROUTINES;++i)if(points[i]==point)++reached[i];
  if(call(&program,&s,&io)!=EXEC_OK)return;
  if(c.resume&&!hw.in_interrupt)return;}
 if(s.words[5]==CM_EXEC_BAILOUT)++alarms[s.words[2]==02642u?0:s.words[2]==02724u?1:2];
 else if(s.words[5]==CM_EXEC_POODOO)++alarms[3];
}
static enum exec_status run_from(apollo_overlay_state *s,cm_native_io *io,cm_exs_case *c){uint16_t steps;
 enum exec_status st;
 hw=c->hw;st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,48,&steps);c->hw=hw;return st;
}
/* A refusal leaves the image as it was. */
static void refused(apollo_overlay_state *s,cm_native_io *io,cm_exs_case *c,const char *what){apollo_overlay_state before=*s;
 check(run_from(s,io,c)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before),what,0);
}
int main(void){apollo_overlay_state s;cm_native_io io;cm_exs_case c;unsigned n,i,kind[3]={0,0,0},dpbit14=0,tc1203=0;uint16_t steps;
 program.basic=&cm_exec_routines;program.basic_count=1;program.hardware=&hw;
 program.operands=cm_exec_site_table(&program.operand_count);
 check(program.operand_count==267u,"267 operand words (253 for 129 service sites, 14 FIXDELAY)",program.operand_count);
 for(n=0;n<CM_EXS_CASES;++n){unsigned k=cm_exs_run(&s,&io,&c,(uint16_t)n,&steps);
  check(cm_exs_digest(&s,(uint16_t)n)==cm_exs_expected[n],"digest",n);
  /* Only LONGCYCL reaching DPBIT14 (from LONGCALL or the task) and the
   * Waitlist insertions that reach TC 1203 are refused. */
  check(k<3u&&(k!=CM_EXS_REFUSED||cm_exs_cohort((uint16_t)n)<=CM_EXS_VARDELAY),"outcome",n);if(k<3u)++kind[k];
  if(k==CM_EXS_REFUSED&&!dpbit14&&cm_exs_cohort((uint16_t)n)==CM_EXS_LONGTASK)dpbit14=n;
  if(k==CM_EXS_REFUSED&&!tc1203&&cm_exs_cohort((uint16_t)n)==CM_EXS_VARDELAY)tc1203=n;
  cover((uint16_t)n);}
 for(i=0;i<ROUTINES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<4u;++i)check(alarms[i]>0u,"alarm reached",i);
 /* Inputs no producer supplies. */
 cm_exs_input(&s,&io,&c,0);s.words[2]=(agc_word)(s.words[2]+2u);refused(&s,&io,&c,"fixed Q off a call site");
 cm_exs_input(&s,&io,&c,0);s.words[2]=07776u;refused(&s,&io,&c,"Q above 07775");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_LONGTASK)+1u);s.words[6]=02002u;s.words[3]=01000u;refused(&s,&io,&c,"LONGCYCL bank");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_LONGTASK)+1u);s.words[5]=CM_EXEC_LNGCALL2;s.words[6]=02002u;s.words[3]=01000u;refused(&s,&io,&c,"LNGCALL2 bank");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_LONGTASK)+1u);s.words[5]=CM_EXEC_LONGRTRN;s.words[CM_EXEC_LONGEXIT]=0;refused(&s,&io,&c,"LONGEXIT 0");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_LONGTASK));s.words[CM_EXEC_LONGCADR]=010000u;refused(&s,&io,&c,"LONGCADR above 07777");
 check(dpbit14!=0u,"a DPBIT14 task case",0);
 cm_exs_input(&s,&io,&c,(uint16_t)dpbit14);refused(&s,&io,&c,"LONGCYCL DPBIT14");
 check(tc1203!=0u,"a TC 1203 case",0);
 cm_exs_input(&s,&io,&c,(uint16_t)tc1203);refused(&s,&io,&c,"VARDELAY TC 1203");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_RESUME));c.hw.in_interrupt=0;refused(&s,&io,&c,"TASKOVER outside T3RUPT");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_RESUME));s.words[0734]=2;refused(&s,&io,&c,"RUPTAGN 2");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_DISPATCH));c.hw.in_interrupt=0;refused(&s,&io,&c,"T3RUPT vector outside an interrupt");
 cm_exs_input(&s,&io,&c,cm_exs_first(CM_EXS_VARDELAY));s.words[0]=0;refused(&s,&io,&c,"VARDELAY zero delay");
 printf("cm-exec-services: %u cases (%u returned, %u alarm, %u refused); routines",CM_EXS_CASES,kind[0],kind[1],kind[2]);
 for(i=0;i<ROUTINES;++i)printf(" %u",reached[i]);
 printf("; alarms 1201 %u 1202 %u 1203 %u 1204 %u; first TC 1203 case %u; %u checks, %u failures\n",alarms[0],alarms[1],alarms[2],alarms[3],tc1203,checks,failures);
 return failures!=0u;
}
