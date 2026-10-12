/* SPDX-License-Identifier: MIT */
#include "cm_lead_tests_cases.h"
#include "standard_lead_routines.h"
#include "cm_native_image.h"
#include "agc_word.h"
static uint32_t rnd(uint32_t *x){*x=*x*1103515245u+12345u;return *x>>8;}
void clt_input(apollo_overlay_state *s,uint16_t *superbank,uint16_t n){uint32_t x=0x1ead5u+n*2654435761u;uint16_t i,eb;
 for(i=0;i<APOLLO_SHARED_WORDS;++i)s->words[i]=(uint16_t)(rnd(&x)&077777u);
 eb=(uint16_t)(rnd(&x)%8u);*superbank=(rnd(&x)&1u)?0060u:0u;
 s->words[0]=(agc_word)(rnd(&x)&0177777u);s->words[1]=agc_extend((agc_word)(rnd(&x)&077777u));s->words[2]=(agc_word)(rnd(&x)&0177777u);
 s->words[3]=(agc_word)(eb<<8);s->words[4]=(agc_word)(033u<<10);s->words[6]=(agc_word)((033u<<10)|eb);s->words[5]=(agc_word)(02000u+n%3u);s->words[7]=0;
}
unsigned clt_run(apollo_overlay_state *s,uint16_t *superbank,uint16_t n){static agc_routine_table t[1];agc_native_program p={0};
 cm_native_io io={0};enum exec_status st;uint16_t steps;
 clt_input(s,superbank,n);t[0]=cm_standard_lead_routines;io.superbank=*superbank;
 p.basic=t;p.basic_count=1;p.interpreter_fetch=AGC_TRANSFER_NONE;
 st=agc_transfer_run(&p,s,&io,superbank,AGC_TRANSFER_NONE,4,&steps);
 return st==EXEC_UNMAPPED&&steps==1u?1u:0u;
}
uint32_t clt_digest(apollo_overlay_state *s,uint16_t n){uint16_t superbank,i,a=0,b=0,v;unsigned k=clt_run(s,&superbank,n);
 a=(uint16_t)(a+k);b=(uint16_t)(b+a);
 if(k)for(i=0;i<APOLLO_SHARED_WORDS;++i){v=(uint16_t)(s->words[i]&(i<3u?0177777u:077777u));a=(uint16_t)(a+v);b=(uint16_t)(b+a);}
 return (uint32_t)b<<16|a;
}
