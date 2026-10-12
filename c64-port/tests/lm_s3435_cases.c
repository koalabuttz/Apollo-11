/* SPDX-License-Identifier: MIT */
#include "lm_s3435_cases.h"
#define W m->words
#define FB35 072000u
static const agc_word entries[LM_S3435_ROUTINES]={03274,03424,03442,03447};
/* The returns each routine's Luminary callers leave: CALL S34/35.1 (P35
 * 35,2753; ELCALC 35,3005), STQ NORMEX of S34/35.3 (its caller NOCHG's
 * CALL at 35,3571), CALL S34/35.4 (S34/35.5 35,3577), CALL LOMAT (S34/35.2
 * 35,3370; S34/35.3 35,3401). */
static const agc_word callers[LM_S3435_ROUTINES][2]={{072754,073006},{073572,073572},{073600,073600},{073371,073402}};
int lm_s3435_owned_newops(agc_word fb,agc_word loc){
 return fb==FB35&&((loc>=03274u&&loc<=03305u)||(loc>=03424u&&loc<=03461u));
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* A caller's return, or a CADR in banks 2-27 (a NEWOPS outside the file). */
static agc_word cadr(uint32_t *seed,unsigned r,unsigned v){
 if(v%4u!=3u)return callers[r][v%2u];
 return (agc_word)(((2u+(rnd(seed)>>16)%026u)<<10)|((rnd(seed)>>8)&01777u));
}
void lm_s3435_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x3435ul+n*7u;unsigned i,r=n%LM_S3435_ROUTINES,v=n/LM_S3435_ROUTINES,vac;agc_word eb;
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)(((rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=FB35;W[6]=(agc_word)(FB35|(eb>>8));
 W[5]=06070;W[0164]=entries[r];W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
 vac=0401u+((v/2u)%5u)*054u;W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
 W[0163]=modes[(v/3u)%3u];
 W[vac+052u]=cadr(&seed,r,v);
 if(r==1u)W[03463]=cadr(&seed,r,v/2u);
 /* One state in eight: the vector UNIT reads zero or nearly so. */
 if(v%8u==5u){static const agc_word at[LM_S3435_ROUTINES]={03536,02261,02261,03536};unsigned a=at[r],k=v&16u?1u:0u;
  if(r==0u)for(i=0;i<6u;++i)W[03552+i]=W[03536+i];
  else for(i=0;i<6u;++i)W[a+i]=(agc_word)(k&&i%2u?(rnd(&seed)>>24)&3u:0u);
  if(r==1u)for(i=0;i<6u;++i)W[02311+i]=W[02261+i];}
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_S3435_NO_TRAMPOLINE
static agc_routine_table interp[1];
static agc_native_program program={0,0,0,0,0,0,LM_S3435_FETCH,0164,0165,0,0,0};
unsigned lm_s3435_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){interp[0]=lm_s3435_routines;program.interpretive=interp;program.interpretive_count=1;}
 lm_s3435_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,16,&steps);
 if(st==EXEC_UNMAPPED)return LM_S3435_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_S3435_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_s3435_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_s3435_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_S3435_STOPPED?1u:kind==LM_S3435_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
