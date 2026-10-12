/* SPDX-License-Identifier: MIT */
#include "lm_s3435c_cases.h"
#ifndef LM_S3435C_NO_TRAMPOLINE
#include "lm_interbank_routines.h"
#define LM_INTERBANK_SITES_MAX 448u
#endif
#define W m->words
#define FB35 072000u
#define INTERPRETIVE 10u
static const agc_word entries[LM_S3435C_ROUTINES]={03306,03367,03371,03522,03531,03566,03572,03576,03600,03604,
 03536,03541,03542,03543,03544,03565,03601,03603};
/* Callers' returns: S34/35.2's (P34's and P35's CALLs, 35,2671 and 2754),
 * S34/35.5's (35,2717 and 2756). */
static const agc_word q_s3435_2[2]={072673,072756},q_s3435_5[2]={072721,072760};
int lm_s3435c_stop(agc_word z,agc_word fb,agc_word q,agc_word loc){
 if(z==06070u)return !(fb==FB35&&((loc>=03274u&&loc<=03326u)||(loc>=03367u&&loc<=03535u)||(loc>=03566u&&loc<=03605u)));
 if(z==02476u)return fb==020000u;   /* GOFLASH, through BANKCALL */
 if(z==05504u||z==06001u||z==06037u)return 1; /* UPFLAG, GOTOPOOH, INTPRET */
 return z==05726u&&q==03404u;       /* POODOO1 from UNIT (alarm 1302) */
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* A real return (from list) three times in four, else a CADR in banks 2-27. */
static agc_word cadr(uint32_t *seed,const agc_word *list,unsigned k,unsigned v){
 if(v%4u!=3u)return list[(v/4u)%k];
 return (agc_word)(((2u+(rnd(seed)>>16)%026u)<<10)|((rnd(seed)>>8)&01777u));
}
void lm_s3435c_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=0x3435cul+n*17u;unsigned i,r=n%LM_S3435C_ROUTINES,v=n/LM_S3435C_ROUTINES,vac;agc_word eb,z=entries[r];
 static const agc_word modes[3]={0,1,077776};
 for(i=0;i<2048u;++i)W[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 eb=(agc_word)(((rnd(&seed)>>20)%8u)<<8);W[3]=eb;W[4]=FB35;W[6]=(agc_word)(FB35|(eb>>8));
 vac=0401u+((v/2u)%5u)*054u;
 if(r<INTERPRETIVE){
  W[5]=06070;W[0164]=z;W[0165]=W[6];W[0115]=040000;W[023]=0;W[067]=0;
  W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%041u);
  W[0163]=modes[(v/3u)%3u];
 }else{
  /* Basic entries: the interpreter's state as debris, Z at the entry. */
  for(i=010;i<=061u;++i)W[i]=0;
  W[5]=z;
 }
 /* PUSHLOC as the callers leave it: room for S34/35.2's 18 pushed words
  * under INTINT's 40D, 18D where LOMAT returns (its SETPD). */
 if(z==03306u)W[0166]=(agc_word)(vac+(rnd(&seed)>>20)%027u);
 if(z==03371u)W[0166]=(agc_word)(vac+022u);
 /* NN zero in one state in two (BHIZ goes straight to INTINT). */
 if(z==03306u&&v%2u==0u)W[03467]=0;
 /* GDT/2 equal to DELVLVC in EBANK7 in one target check in two (no new
  * target). */
 if(v%2u==1u)for(i=0;i<6u;++i)W[01236u+i]=W[03433u+i];
 W[vac+052u]=z<03522u?cadr(&seed,q_s3435_2,2,v):cadr(&seed,q_s3435_5,2,v);
 W[03470]=z<03522u?cadr(&seed,q_s3435_2,2,v/4u):cadr(&seed,q_s3435_5,2,v/4u);
 io->superbank=(uint16_t)(((v/16u)%4u)*020u);io->interrupt_enabled=(uint16_t)((v/64u)&1u);
}
#ifndef LM_S3435C_NO_TRAMPOLINE
#if !defined LM_S3435C_C64_GROUP || LM_S3435C_C64_GROUP>=3
/* lm_interbank_sites and lm-p34's TCR sites merged by address. */
static agc_site_operand operands[LM_INTERBANK_SITES_MAX+LM_P34_SITES];
static const agc_site_operand *merged(uint16_t *count){static uint16_t total;
 if(!total){uint16_t a,b,i=0,j=0;const agc_site_operand *x=lm_interbank_site_table(&a),*y=lm_p34_site_table(&b);
  if(a>LM_INTERBANK_SITES_MAX){*count=0;return operands;}
  while(i<a||j<b)operands[total++]=(j>=b||(i<a&&x[i].physical<y[j].physical))?x[i++]:y[j++];}
 *count=total;return operands;
}
#endif
static agc_routine_table interp[3];
static agc_native_program program={0,0,0,0,0,0,LM_S3435C_FETCH,0164,0165,0,0,0};
/* The tables a run reaches: C64 group 1 (S34/35.2) lm-s3435b's INTINT,
 * group 2 (its LOMAT return) nothing else, group 3 the basic tables. */
#ifndef LM_S3435C_C64_GROUP
#define WITH_BASIC 1
#define WITH_S3435B 1
#define WITH_S3435 1
#else
#define WITH_BASIC (LM_S3435C_C64_GROUP==3)
#define WITH_S3435B (LM_S3435C_C64_GROUP==1)
#define WITH_S3435 0
#endif
unsigned lm_s3435c_run(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){enum exec_status st;uint16_t steps;
 if(!program.interpretive){uint8_t k=0;
#if WITH_BASIC
  static agc_routine_table basic[3];
  basic[0]=lm_s3435c_basic_routines;basic[1]=lm_p34_basic_routines;basic[2]=lm_interbank_routines;
  program.basic=basic;program.basic_count=3;program.operands=merged(&program.operand_count);
#endif
  interp[k++]=lm_s3435c_interpretive_routines;
#if WITH_S3435B
  interp[k++]=lm_s3435b_routines;
#endif
#if WITH_S3435
  interp[k++]=lm_s3435_routines;
#endif
  program.interpretive=interp;program.interpretive_count=k;}
 lm_s3435c_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,64,&steps);
 if(st==EXEC_UNMAPPED)return LM_S3435C_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LM_S3435C_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_s3435c_digest(apollo_overlay_state *s,uint32_t n){p66_bias_io io;unsigned i,kind=lm_s3435c_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)(kind==LM_S3435C_STOPPED?1u:kind==LM_S3435C_REFUSED?0u:3u));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 mix(io.interrupt_enabled);
 return (uint32_t)sum_b<<16|sum_a;
}
#endif
