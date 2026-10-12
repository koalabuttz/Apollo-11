/* SPDX-License-Identifier: MIT */
#include "lm_p30_cases.h"
#define W m->words
#define FB34 070000u
#define FB35 072000u
/* Entry FBANK and Z or LOC. */
static const agc_word banks[LM_P30_ROUTINES]={FB34,FB34,FB34,FB34,FB34,FB34,FB34,FB35,FB35,FB35,
 FB35,FB35,FB35,FB35,FB35,FB35,FB35,FB35};
static const agc_word entries[LM_P30_ROUTINES]={02000,02005,02043,02045,02047,02053,02055,02013,02015,02022,
 02000,02002,02004,02006,02010,02012,02017,02021};
int lm_p30_stop(agc_word z,agc_word fb,agc_word q,agc_word loc){
 if(z==06070u)return !((fb==FB34&&loc>=02000u&&loc<=02056u)||(fb==FB35&&((loc>=02013u&&loc<=02016u)||(loc>=02022u&&loc<=02024u))));
 if(fb==FB35&&z==03712u)return 1;                         /* VNPOOH */
 if(z==05504u||z==05516u||z==06037u)return 1;             /* UPFLAG, DOWNFLAG, INTPRET */
 return z==05726u&&q==03404u;                             /* POODOO1 (alarm 1302) */
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* P30's return from S30.1 three times in four, else a CADR in banks 2-27. */
static agc_word cadr(uint32_t *seed,unsigned v){
 if(v%4u!=3u)return 072015u;
 return (agc_word)(((2u+(rnd(seed)>>16)%026u)<<10)|((rnd(seed)>>8)&01777u));
}
void lm_p30_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x30037ul+n*41u;unsigned i,r=n%LM_P30_ROUTINES,v=n/LM_P30_ROUTINES,vac;agc_word eb,fb=banks[r],z=entries[r];
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)((v%4u!=3u?7u:(rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=fb;W[6]=(agc_word)(fb|(eb>>8));
 vac=0401u+((v/2u)%5u)*054u;
 if(r<LM_P30_INTERPRETIVE){
  W[5]=06070;W[0164]=z;W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
  W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
  W[0163]=modes[(v/3u)%3u];
 }else{
  /* Basic entries: the interpreter's state as debris, Z at the entry. */
  for(i=010;i<=061u;++i)W[i]=0;
  W[5]=z;
 }
 W[vac+052u]=cadr(&seed,v);W[03632]=cadr(&seed,v/4u);
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_P30_NO_TRAMPOLINE
static agc_routine_table interp[1],basic[1];
static agc_native_program program={0,0,0,0,0,0,LM_P30_FETCH,0164,0165,0,0,0};
unsigned lm_p30_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){
  basic[0]=lm_p30_basic_routines;program.basic=basic;program.basic_count=1;
  interp[0]=lm_p30_interpretive_routines;program.interpretive=interp;program.interpretive_count=1;}
 lm_p30_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,64,&steps);
 if(st==EXEC_UNMAPPED)return LM_P30_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_P30_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_p30_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_p30_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_P30_STOPPED?1u:kind==LM_P30_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
