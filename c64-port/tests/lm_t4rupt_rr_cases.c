/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_t4rupt_rr_cases.h"
static const agc_word entry_z[LM_T4RUPT_RR_ENTRIES]={03006,03032,03070,03071,03113,03114,03126,03132,03134,03137,03142,03151};
agc_word lrr_entry(uint16_t n){return n<LRR_FIRST_CASES?entry_z[0]:entry_z[1u+(n-LRR_FIRST_CASES)/LRR_PER_ENTRY];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
void lrr_input(apollo_overlay_state *m,lrr_io *io,uint16_t n){uint32_t r=0x06300600u+n*2654435761u;unsigned i,k;agc_word eb,z=lrr_entry(n);
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054u;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 /* EB as T4RPTBB sets it (E6) in most cases, any bank in the rest. */
 eb=(agc_word)(rnd(&r)%4u?6u:rnd(&r)%8u);m->words[3]=(agc_word)(eb<<8);m->words[4]=014000u;m->words[6]=(agc_word)(014000u|eb);m->words[5]=z;
 m->words[2]=(agc_word)(06000u+rnd(&r)%0700u);
 /* The flag words with few bits set in half the cases, so each test goes both ways. */
 k=rnd(&r)%2u;
 if(k){m->words[0110]=(agc_word)(rnd(&r)&rnd(&r)&AGC_MASK);m->words[074]&=(agc_word)rnd(&r);m->words[0101]&=(agc_word)rnd(&r);
  m->words[0102]&=(agc_word)rnd(&r);m->words[0103]&=(agc_word)rnd(&r);}
 /* SPSIN and SPCOS leave A without overflow. */
 if(z==03134u||z==03137u||z==03142u||z==03151u)m->words[0]=agc_extend((agc_word)(rnd(&r)&AGC_MASK));
 memset(io,0,sizeof *io);
 io->lm.superbank=(agc_word)((rnd(&r)%8u)*020u);io->lm.interrupt_enabled=0;io->hw.in_interrupt=1;
 io->hw.channel11=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel12=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel14=(agc_word)(rnd(&r)&AGC_MASK)&(agc_word)~077000u;
 io->hw.channel30=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel33=(agc_word)(rnd(&r)&AGC_MASK);
 /* Channels 30 and 33 agreeing with RADMODES in half the cases. */
 if(rnd(&r)%2u)io->hw.channel33=(agc_word)((io->hw.channel33&~02u)|(m->words[0110]&02u));
 if(rnd(&r)%2u)io->hw.channel30=(agc_word)((io->hw.channel30&~0100u)|(m->words[0110]&0100u));
}
unsigned lrr_run(apollo_overlay_state *s,lrr_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&lm_t4rupt_rr_routines;program.basic_count=1;program.hardware=&io->hw;
 lrr_input(s,io,n);
 call=agc_transfer_resolve(&program,s,io->lm.superbank);
 if(!call)return 3u;
 return call(&program,s,&io->lm)==EXEC_OK?LRR_RETURNED:LRR_REFUSED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lrr_digest(apollo_overlay_state *s,uint16_t n){static lrr_io io;unsigned i,kind=lrr_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LRR_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.lm.superbank);mix(io.hw.channel11);mix(io.hw.channel12);mix(io.hw.channel14);mix(io.hw.channel30);mix(io.hw.channel33);}
 return (uint32_t)sum_b<<16|sum_a;
}
