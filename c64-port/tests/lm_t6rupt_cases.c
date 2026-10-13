/* SPDX-License-Identifier: MIT */
#include "lm_t6rupt_cases.h"
static const uint16_t start[LM_T6_COHORTS+1]={0,256,1792,2304,3072,3840};
enum lm_t6_cohort lm_t6_cohort(uint16_t n){unsigned k=0;while(k+1u<LM_T6_COHORTS&&n>=start[k+1u])++k;return (enum lm_t6_cohort)k;}
uint16_t lm_t6_first(enum lm_t6_cohort k){return start[k];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
#define E6(a) (03000u+((a)-01400u))
/* T6 schedule times: below and at the PRIO37 overflow (01000), large, idle. */
static agc_word time6(uint32_t *r){static const agc_word t[8]={0,1,0777,01000,01001,07777,037776,037777};uint32_t x=rnd(r);return x&8u?t[x&7u]:(agc_word)((x>>4)&01777u);}
static agc_word index6(uint32_t *r){static const agc_word x[3]={0,4,13};return x[rnd(r)%3u];}
static agc_word bank(uint32_t *r){return (agc_word)((rnd(r)%32u)*02000u);}
/* A return address no entry's path passes through. */
static agc_word ret(uint32_t *r){uint32_t x=rnd(r);return (agc_word)(x&1u?06000u+(x>>1)%02000u:02000u+(x>>1)%02000u);}
void lm_t6_input(apollo_overlay_state *m,p66_bias_io *io,agc_hardware *hw,uint16_t n){enum lm_t6_cohort k=lm_t6_cohort(n);uint16_t j=(uint16_t)(n-start[k]);uint32_t r=0x06031099u+n*2654435761u;unsigned i;agc_word fb;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=0;m->words[035]=0;m->words[036]=0;m->words[047]=0;for(i=050;i<=054;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);m->words[2]=(agc_word)(rnd(&r)&07777u);
 fb=k==LM_T6_DOT6RUPT||k==LM_T6_T6JOBCHK?LM_T6_BANK:bank(&r);
 m->words[3]=03000;m->words[4]=fb;m->words[6]=(agc_word)(fb|6u);
 m->words[E6(LM_T6NEXT)]=time6(&r);m->words[E6(LM_T6NEXT)+1u]=index6(&r);
 m->words[E6(LM_T6FURTHA)]=time6(&r);m->words[E6(LM_T6FURTHA)+1u]=index6(&r);m->words[E6(LM_NXT6ADR)]=index6(&r);
 /* TIME6: mostly -0 (a T6RUPT is due), else counting (+: one to three
  * counts from its end, or full scale, or anywhere) or +0. */
 {uint32_t x=rnd(&r)%8u,y;agc_word t;
  if(x==5u){y=rnd(&r)%4u;t=(agc_word)(y<3u?1u+y:037777u);}else t=(agc_word)(x<5u?077777u:x<7u?1u+rnd(&r)%037776u:0u);
  m->words[LM_TIME6]=t;}
 io->superbank=(agc_word)((rnd(&r)%8u)*020u);io->interrupt_enabled=(agc_word)(rnd(&r)&1u);
 hw->channel11=0;hw->channel5=(agc_word)(rnd(&r)&AGC_MASK);hw->channel6=(agc_word)(rnd(&r)&AGC_MASK);hw->channel13=(agc_word)(rnd(&r)&043777u);
 hw->in_interrupt=1;
 switch(k){
 case LM_T6_VECTOR:m->words[5]=LM_T6RUPT_VECTOR;break;
 case LM_T6_DOT6RUPT:m->words[5]=LM_DOT6RUPT;break;
 case LM_T6_T6JOBCHK:m->words[5]=LM_T6JOBCHK;m->words[2]=(agc_word)(06000u+rnd(&r)%02000u);hw->in_interrupt=(uint16_t)(j&1u);break;
 case LM_T6_WRITE:{static const agc_word z[6]={LM_WRITEP-1u,LM_WRITEP,LM_WRITEU-1u,LM_WRITEU,LM_WRITEV-1u,LM_WRITEV};
  m->words[5]=z[j%6u];m->words[2]=ret(&r);m->words[0]=(agc_word)(rnd(&r)&0377u);hw->in_interrupt=(uint16_t)((j/6u)&1u);break;}
 default:{static const agc_word z[3]={LM_RESUME,LM_NOQRSM,LM_NOQBRSM};
  m->words[5]=z[j%3u];m->words[015]=(agc_word)(1u+rnd(&r)%010000u);m->words[016]=(agc_word)(bank(&r)|((rnd(&r)%8u)*020u)|(rnd(&r)&7u));break;}
 }
}
unsigned lm_t6_run(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&lm_t6rupt_routines;program.basic_count=1;program.hardware=hw;
 lm_t6_input(s,io,hw,n);
 call=agc_transfer_resolve(&program,s,io->superbank);
 if(!call)return 3u;
 return call(&program,s,io)==EXEC_OK?LM_T6_RETURNED:LM_T6_REFUSED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_t6_digest(apollo_overlay_state *s,uint16_t n){p66_bias_io io;agc_hardware hw;unsigned i,kind=lm_t6_run(s,&io,&hw,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LM_T6_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.superbank);mix(io.interrupt_enabled);mix(hw.in_interrupt);mix(hw.channel5);mix(hw.channel6);mix(hw.channel13);}
 return (uint32_t)sum_b<<16|sum_a;
}
