/* SPDX-License-Identifier: MIT */
#include "interbank_q_cases.h"
#include "lm_interbank_routines.h"
#include "cm_interbank_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"
static uint32_t rnd(uint32_t *x){*x=*x*1103515245u+12345u;return *x>>8;}
uint16_t ibq_entry(uint16_t n){static const uint16_t z[4]={04621u,04665u,04640u,04704u};return z[n%4u];}
void ibq_input(apollo_overlay_state *s,uint16_t *superbank,uint16_t n){uint32_t x=0x5eedu+n*2654435761u;uint16_t i,bank,fb,eb;
 for(i=0;i<APOLLO_SHARED_WORDS;++i)s->words[i]=(uint16_t)(rnd(&x)&077777u);
 bank=(uint16_t)(4u+rnd(&x)%034u);fb=(uint16_t)(rnd(&x)%040u);eb=(uint16_t)(rnd(&x)%8u);
 *superbank=(rnd(&x)&1u)?0100u:0060u;
 s->words[0]=(agc_word)((bank<<10)|(rnd(&x)&01777u));s->words[1]=(agc_word)(rnd(&x)&077777u);
 if(n%4u<2u)s->words[2]=(agc_word)(0100u+rnd(&x)%(07776u-0100u));
 else s->words[2]=(agc_word)((n/4u)%2u?rnd(&x)%0100u:0100u+rnd(&x)%(07776u-0100u));
 s->words[3]=(agc_word)(eb<<8);s->words[4]=(agc_word)(fb<<10);s->words[6]=(agc_word)((fb<<10)|eb);s->words[5]=ibq_entry(n);s->words[7]=0;
}
unsigned ibq_run(apollo_overlay_state *s,uint16_t *superbank,uint16_t n){static agc_routine_table t[1];agc_native_program p={0};
 p66_bias_io lio={0};cm_native_io cio={0};void *io;enum exec_status st;uint16_t steps;
 ibq_input(s,superbank,n);
 if(n%2u){t[0]=cm_interbank_routines;cio.superbank=*superbank;io=&cio;}else{t[0]=lm_interbank_routines;lio.superbank=*superbank;io=&lio;}
 p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;
 st=agc_transfer_run(&p,s,io,superbank,AGC_TRANSFER_NONE,IBQ_MAX_STEPS,&steps);
 return st==EXEC_UNMAPPED?1u:0u;
}
uint32_t ibq_digest(apollo_overlay_state *s,uint16_t n){uint16_t superbank,i,a=0,b=0,v;unsigned k=ibq_run(s,&superbank,n);
 a=(uint16_t)(a+k);b=(uint16_t)(b+a);
 if(k)for(i=0;i<APOLLO_SHARED_WORDS;++i){v=(uint16_t)(s->words[i]&(i<3u?0177777u:077777u));a=(uint16_t)(a+v);b=(uint16_t)(b+a);}
 return (uint32_t)b<<16|a;
}
