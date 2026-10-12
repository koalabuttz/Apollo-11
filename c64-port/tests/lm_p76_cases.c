/* SPDX-License-Identifier: MIT */
#include "lm_p76_cases.h"
#define W m->words
#define FB13 026000u
static const agc_word entries[LM_P76_ROUTINES]={02212,02233,02237,02261,02263,02302,02304,02315,02323,02325,02332,02340,
 02207,02211,02216,02221,02222,02223,02224,02227,02230,02231,02232,02316,02320,02322,02330,02333};
int lm_p76_stop(agc_word z,agc_word fb,agc_word q,agc_word loc){
 if(z==06070u)return !(fb==FB13&&((loc>=02212u&&loc<=02215u)||(loc>=02233u&&loc<=02315u)||(loc>=02323u&&loc<=02332u)||(loc>=02340u&&loc<=02350u)));
 /* UPFLAG, PHASCHNG, BANKCALL, GOTOPOOH, INTPRET */
 if(z==05504u||z==05353u||z==04616u||z==06001u||z==06037u)return 1;
 return z==05726u&&q==03404u;                             /* POODOO1 (alarm 1302) */
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void lm_p76_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x76076ul+n*37u;unsigned i,r=n%LM_P76_ROUTINES,v=n/LM_P76_ROUTINES,vac;agc_word eb,z=entries[r];
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)((v%4u!=3u?7u:(rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=FB13;W[6]=(agc_word)(FB13|(eb>>8));
 vac=0401u+((v/2u)%5u)*054u;
 if(r<LM_P76_INTERPRETIVE){
  W[5]=06070;W[0164]=z;W[0165]=W[6];W[0115]=0;W[023]=0;W[067]=0;
  W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
  W[0163]=modes[(v/3u)%3u];
 }else{
  /* Basic entries: the interpreter's state as debris, Z at the entry. */
  for(i=010;i<=061u;++i)W[i]=0;
  W[5]=z;
 }
 /* QPRET: P76SUB1's real return (13,2263) three times in four, else a CADR
  * in banks 2-27 other than 13. */
 if(v%4u!=3u)W[vac+052u]=026263u;
 else{unsigned b=2u+(rnd(&seed)>>16)%025u;W[vac+052u]=(agc_word)(((b>=013u?b+1u:b)<<10)|((rnd(&seed)>>8)&01777u));}
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_P76_NO_TRAMPOLINE
static agc_routine_table interp[1],basic[1];
static agc_native_program program={0,0,0,0,0,0,LM_P76_FETCH,0164,0165,0,0,0};
unsigned lm_p76_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){
  basic[0]=lm_p76_basic_routines;program.basic=basic;program.basic_count=1;
  interp[0]=lm_p76_interpretive_routines;program.interpretive=interp;program.interpretive_count=1;}
 lm_p76_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,64,&steps);
 if(st==EXEC_UNMAPPED)return LM_P76_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_P76_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_p76_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_p76_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_P76_STOPPED?1u:kind==LM_P76_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
