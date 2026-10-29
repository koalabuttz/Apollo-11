/* SPDX-License-Identifier: MIT */
#include "lm_r31_cases.h"
#define W m->words
#define FB37 076000u
#define FB30 060000u
static const agc_word entries[LM_R31_ROUTINES]={03132,03134,03136,03144,03147,03166,03170,03172,03211,03217,03222,
 03240,03246,03263,03304,03331,03347,03355,03363,03365,03371,03373,03375,03404,
 03113,03116,03117,03120,03121,03155,03316,03326,03330,03344,03346,03370,03377,03403,
 03674,03700,03702,03712};
int lm_r31_stop(agc_word z,agc_word fb,agc_word q,agc_word loc){
 /* The split points (COMPDISP, RANGE and RRATE, the second REFSMMAT
  * rotation): entries of their own, and stops for runs that reach them. */
 if(z==06070u&&fb==FB37&&(loc==03240u||loc==03246u||loc==03355u))return 1;
 if(z==06070u)return !(fb==FB37&&((loc>=03132u&&loc<=03154u)||(loc>=03166u&&loc<=03315u)||(loc>=03331u&&loc<=03343u)||
  (loc>=03347u&&loc<=03367u)||(loc>=03371u&&loc<=03376u)||(loc>=03404u&&loc<=03406u)));
 if(fb==020000u&&z==03573u)return 1;                      /* LOADTIME (10,3573) */
 if(fb==0u&&(z==03612u||z==03402u))return 1;              /* ARCCOS, UNIT's 1302 exit */
 /* From bank 40: FINDVAC, FIXDELAY, NOVAC, TASKOVER. From bank 37:
  * BANKCALL, B5OFF, PRIOCHNG, ENDEXT, INTPRET (NORMUNX1's own BANKCALL
  * runs in bank 10). */
 if(fb==FB30&&(z==05105u||z==05221u||z==05072u||z==05261u))return 1;
 if(fb==FB37&&(z==04616u||z==05563u||z==05146u||z==05472u||z==06037u))return 1;
 return z==05726u&&q==03404u;                             /* POODOO1 (alarm 1302) */
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void lm_r31_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x31037ul+n*37u;unsigned i,r=n%LM_R31_ROUTINES,v=n/LM_R31_ROUTINES,vac;agc_word eb,fb,z=entries[r];
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 fb=r>=LM_R31_ROUTINES-4u?FB30:FB37;
 eb=(agc_word)((v%4u!=3u?7u:(rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=fb;W[6]=(agc_word)(fb|(eb>>8));
 vac=0401u+((v/2u)%5u)*054u;
 if(r<LM_R31_INTERPRETIVE){
  W[5]=06070;W[0164]=z;W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
  W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
  W[0163]=modes[(v/3u)%3u];
 }else{
  /* Basic entries: the interpreter's state as debris, Z at the entry. */
  for(i=010;i<=061u;++i)W[i]=0;
  W[5]=z;
 }
 io->superbank=fb==FB30?0100u:(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_R31_NO_TRAMPOLINE
static agc_routine_table interp[1],basic[1];
static agc_native_program program={0,0,0,0,0,0,LM_R31_FETCH,0164,0165,0,0,0};
/* The split point a run from entry z can reach (lm_r31_stop): COMPDISP
 * reaches RANGE and RRATE, GETRVN's first rotation the second, and the
 * second COMPDISP. */
static uint16_t split(agc_word z){
 return z==03240u?LM_R31_B37(03246):z==03347u?LM_R31_B37(03355):z==03355u?LM_R31_B37(03240):0xfffeu;
}
unsigned lm_r31_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){
  basic[0]=lm_r31_basic_routines;program.basic=basic;program.basic_count=1;
  interp[0]=lm_r31_interpretive_routines;program.interpretive=interp;program.interpretive_count=1;}
 lm_r31_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,n%LM_R31_ROUTINES<LM_R31_INTERPRETIVE?split(entries[n%LM_R31_ROUTINES]):0xfffeu,64,&steps);
 /* EXEC_OK: the run reached its split point. */
 if(st==EXEC_UNMAPPED||st==EXEC_OK)return LM_R31_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_R31_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_r31_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_r31_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_R31_STOPPED?1u:kind==LM_R31_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
