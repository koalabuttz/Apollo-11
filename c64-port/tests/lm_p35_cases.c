/* SPDX-License-Identifier: MIT */
#include "lm_p35_cases.h"
#define W m->words
#define FB35 072000u
static const agc_word entries[LM_P35_ROUTINES]={02737,02741,02743,02752,02754,02756,02760,02762,
 02725,02726,02731,02732,02736};
int lm_p35_stop(agc_word z,agc_word fb,agc_word loc){
 if(z==06070u)return !(fb==FB35&&loc>=02737u&&loc<=02763u);
 if(fb==FB35&&(z==02347u||z==02354u||z==02361u))return 1; /* AVFLAGA, AVFLAGP, P20FLGON */
 if(z==06037u)return 1;                                     /* INTPRET */
 return z==03573u&&fb==020000u;                             /* LOADTIME (10,3573) */
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void lm_p35_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x35035ul+n*29u;unsigned i,r=n%LM_P35_ROUTINES,v=n/LM_P35_ROUTINES,vac;agc_word eb,z=entries[r];
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)((v%4u!=3u?7u:(rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=FB35;W[6]=(agc_word)(FB35|(eb>>8));
 vac=0401u+((v/2u)%5u)*054u;
 if(r<LM_P35_INTERPRETIVE){
  W[5]=06070;W[0164]=z;W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
  W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
  W[0163]=modes[(v/3u)%3u];
 }else{
  /* Basic entries: the interpreter's state as debris, Z at the entry. */
  for(i=010;i<=061u;++i)W[i]=0;
  W[5]=z;
 }
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_P35_NO_TRAMPOLINE
static agc_routine_table interp[1],basic[1];
static agc_native_program program={0,0,0,0,0,0,LM_P35_FETCH,0164,0165,0,0,0};
unsigned lm_p35_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){
  basic[0]=lm_p35_basic_routines;program.basic=basic;program.basic_count=1;
  interp[0]=lm_p35_interpretive_routines;program.interpretive=interp;program.interpretive_count=1;}
 lm_p35_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,64,&steps);
 if(st==EXEC_UNMAPPED)return LM_P35_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_P35_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_p35_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_p35_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_P35_STOPPED?1u:kind==LM_P35_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
