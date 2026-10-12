/* SPDX-License-Identifier: MIT */
#include "cm_p21_cases.h"
#define W m->words
#define FB37 076000u
static const agc_word entries[CM_P21_ROUTINES]={02020,02024,02035,02042,02044,02065,02104,02110,02125,02137,
 02001,02003,02006,02007,02010,02011,02014,02015,02016,02017,02131,02134,02135,02136};
int cm_p21_stop(agc_word z,agc_word fb,agc_word q,agc_word loc){
 if(z==06037u)return !(fb==FB37&&((loc>=02020u&&loc<=02130u)||(loc>=02137u&&loc<=02144u)));
 if(fb==0u&&z==03607u)return 1;                           /* ARCSIN */
 /* BANKCALL, GOTOPOOH, INTPRET */
 if(z==04662u||z==04106u||z==06006u)return 1;
 return z==05721u&&q==03404u;                             /* POODOO (alarm 1302) */
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_p21_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x21037ul+n*37u;unsigned i,r=n%CM_P21_ROUTINES,v=n/CM_P21_ROUTINES,vac;agc_word eb,z=entries[r];
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)((v%4u!=3u?4u:(rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=FB37;W[6]=(agc_word)(FB37|(eb>>8));
 vac=0401u+((v/2u)%5u)*054u;
 if(r<CM_P21_INTERPRETIVE){
  W[5]=06037;W[0164]=z;W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
  W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
  W[0163]=modes[(v/3u)%3u];
 }else{
  /* Basic entries: the interpreter's state as debris, Z at the entry. */
  for(i=010;i<=061u;++i)W[i]=0;
  W[5]=z;
 }
 /* OPTION2, P21ORIG (E7,1603) and X2 zero in one case
  * in two each. */
 if(v%2u==0u)W[01132]=0;
 if((v/2u)%2u==0u)W[03603]=0;
 if((v/4u)%2u==0u)W[vac+047u]=0;
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef CM_P21_NO_TRAMPOLINE
static agc_routine_table interp[1],basic[1];
static agc_native_program program={0,0,0,0,0,0,CM_P21_FETCH,0164,0165,0,0,0};
unsigned cm_p21_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){
  basic[0]=cm_p21_basic_routines;program.basic=basic;program.basic_count=1;
  interp[0]=cm_p21_interpretive_routines;program.interpretive=interp;program.interpretive_count=1;}
 cm_p21_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,64,&steps);
 if(st==EXEC_UNMAPPED)return CM_P21_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return CM_P21_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_p21_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=cm_p21_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==CM_P21_STOPPED?1u:kind==CM_P21_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
