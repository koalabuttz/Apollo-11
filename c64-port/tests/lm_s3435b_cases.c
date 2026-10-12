/* SPDX-License-Identifier: MIT */
#include "lm_s3435b_cases.h"
#define W m->words
#define FB35 072000u
static const agc_word entries[LM_S3435B_ROUTINES]={03377,03402,03421,03462,03466,03471,03517};
/* Callers' returns: S34/35.3's QPRET (NOCHG's CALL, 35,3571), NORMEX as it
 * holds S34/35.3's caller, GOINT's (ELCALC's CALLs 35,3245 and 3254),
 * INTINT's RTRN (GOINT's two, S34/35.2's CALL 35,3326, S34/35.3's 3420). */
static const agc_word q_s3435_3[1]={073572},q_goint[2]={073246,073255},q_rtrn[4]={073246,073255,073327,073421};
int lm_s3435b_owned_newops(agc_word fb,agc_word loc){
 return fb==FB35&&((loc>=03274u&&loc<=03305u)||(loc>=03377u&&loc<=03521u));
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* A real return (from list) three times in four, else a CADR in banks 2-27. */
static agc_word cadr(uint32_t *seed,const agc_word *list,unsigned k,unsigned v){
 if(v%4u!=3u)return list[(v/4u)%k];
 return (agc_word)(((2u+(rnd(seed)>>16)%026u)<<10)|((rnd(seed)>>8)&01777u));
}
void lm_s3435b_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x3435bul+n*13u;unsigned i,r=n%LM_S3435B_ROUTINES,v=n/LM_S3435B_ROUTINES,vac;agc_word eb;
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)(((rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=FB35;W[6]=(agc_word)(FB35|(eb>>8));
 W[5]=06070;W[0164]=entries[r];W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
 vac=0401u+((v/2u)%5u)*054u;W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
 W[0163]=modes[(v/3u)%3u];
 /* PUSHLOC as the callers leave it: 18D where LOMAT returns (its SETPD),
  * room for GOINT's four pushes. */
 if(r==1u)W[0166]=(agc_word)(vac+022u);
 if(r==3u)W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%031u);
 /* Where INTSTALL returns, one state in four has the time its caller
  * pushed last zero (BZE takes +2). */
 if(r==5u&&v%4u==1u){W[0166]=(agc_word)(vac+2u+(rnd(&seed)>>20)%037u);W[W[0166]-1u]=0;W[W[0166]-2u]=0;}
 W[vac+052u]=r==3u?cadr(&seed,q_goint,2,v):cadr(&seed,q_s3435_3,1,v);
 W[03463]=cadr(&seed,q_s3435_3,1,v/4u);
 W[03465]=cadr(&seed,q_rtrn,4,v/2u);
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_S3435B_NO_TRAMPOLINE
static agc_routine_table interp[2];
static agc_native_program program={0,0,0,0,0,0,LM_S3435B_FETCH,0164,0165,0,0,0};
unsigned lm_s3435b_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){interp[0]=lm_s3435b_routines;program.interpretive=interp;
#if defined LM_S3435B_C64_GROUP && LM_S3435B_C64_GROUP<=2
  /* Groups 1 and 2 reach none of lm-s3435's routines. */
  program.interpretive_count=1;
#else
  interp[1]=lm_s3435_routines;program.interpretive_count=2;
#endif
 }
 lm_s3435b_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,16,&steps);
 if(st==EXEC_UNMAPPED)return LM_S3435B_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_S3435B_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_s3435b_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_s3435b_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_S3435B_STOPPED?1u:kind==LM_S3435B_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
