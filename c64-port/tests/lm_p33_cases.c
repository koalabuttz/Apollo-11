/* SPDX-License-Identifier: MIT */
#include "lm_p33_cases.h"
#define W m->words
#define FB35 072000u
static const agc_word entries[LM_P33_ROUTINES]={02225,02233,02235,02237,02245,02247,02255,02257,02264,02277,
 02302,02306,02314,02325,02340,02343,02345,02370,02374,02406,
 02216,02217,02220,02221,02222,02224,02266,02270,02273,02274,02275,02276,02335,02337,02347,02353,02354,02360,
 02361,02365,02367,02403,02405};
int lm_p33_stop(agc_word z,agc_word fb,agc_word q,agc_word loc){
 if(z==06070u)return !(fb==FB35&&((loc>=02225u&&loc<=02265u)||(loc>=02277u&&loc<=02346u)||(loc>=02370u&&loc<=02402u)||(loc>=02406u&&loc<=02407u)));
 if(fb==FB35&&z==03712u)return 1;                         /* VNPOOH */
 /* UPFLAG, DOWNFLAG, ALARM, BANKCALL, GOTOPOOH, INTPRET, SUBEXIT */
 if(z==05504u||z==05516u||z==05567u||z==04616u||z==06001u||z==06037u||z==01470u)return 1;
 return z==05726u&&q==03404u;                             /* POODOO1 (alarm 1302) */
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void lm_p33_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x33073ul+n*47u;unsigned i,r=n%LM_P33_ROUTINES,v=n/LM_P33_ROUTINES,vac;agc_word eb,fb=FB35,z=entries[r];
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)((v%4u!=3u?7u:(rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=fb;W[6]=(agc_word)(fb|(eb>>8));
 vac=0401u+((v/2u)%5u)*054u;
 if(r<LM_P33_INTERPRETIVE){
  W[5]=06070;W[0164]=z;W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
  W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
  W[0163]=modes[(v/3u)%3u];
 }else{
  /* Basic entries: the interpreter's state as debris, Z at the entry. */
  for(i=010;i<=061u;++i)W[i]=0;
  W[5]=z;
 }
 /* MPAC zero in one S33/34.1 return in two; NORMEX a CADR in banks 2-27. */
 if(z==02264u&&v%2u==0u)W[0154]=W[0155]=W[0156]=0;
 W[03463]=(agc_word)(((2u+(rnd(&seed)>>16)%026u)<<10)|((rnd(&seed)>>8)&01777u));
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_P33_NO_TRAMPOLINE
static agc_routine_table interp[1],basic[1];
static agc_native_program program={0,0,0,0,0,0,LM_P33_FETCH,0164,0165,0,0,0};
unsigned lm_p33_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){
  basic[0]=lm_p33_basic_routines;program.basic=basic;program.basic_count=1;
  interp[0]=lm_p33_interpretive_routines;program.interpretive=interp;program.interpretive_count=1;}
 lm_p33_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,64,&steps);
 if(st==EXEC_UNMAPPED)return LM_P33_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_P33_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_p33_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_p33_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_P33_STOPPED?1u:kind==LM_P33_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
