/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_t4rupt_chain_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
void lch_input(apollo_overlay_state *m,lch_io *io,uint16_t n){uint32_t r=0x04c4a140u+n*2654435761u;unsigned i,k;agc_word eb,x;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054u;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 /* T4RUPT as its vector enters it: FB 06, EB 6 (T4RPTBB) in most cases. */
 eb=(agc_word)(rnd(&r)%4u?6u:rnd(&r)%8u);m->words[3]=(agc_word)(eb<<8);m->words[4]=014000u;m->words[6]=(agc_word)(014000u|eb);m->words[5]=02000u;
 /* DSRUPTSW: a phase count (each T4JUMP target), +0, or the quick display. */
 x=(agc_word)(rnd(&r)%8u);m->words[01313]=(agc_word)(x<5u?rnd(&r)%9u:x==5u?0u:(rnd(&r)&037777u)|040000u);
 if(x==7u)m->words[01313]|=0377u;
 m->words[0776]=(agc_word)(rnd(&r)%12u);if(m->words[0776]==013u)m->words[0776]=077777u;
 for(i=0;i<12u;++i){x=(agc_word)(rnd(&r)&AGC_MASK);if(x==077777u||rnd(&r)%4u)x&=037777u;if(x==0u&&i<11u)x=1u;m->words[01023+i]=x;}
 if(rnd(&r)%8u==0u)m->words[01036]=0;
 x=(agc_word)(rnd(&r)%4u);m->words[01016]=(agc_word)(x==0u?0u:x==1u?rnd(&r)%013u:rnd(&r)&AGC_MASK);
 /* IMODES30/33, RADMODES and the flag words near flight values in half the cases. */
 k=rnd(&r)%2u;
 if(k){m->words[01302]=(agc_word)(037411u^(rnd(&r)&076777u&(rnd(&r)|rnd(&r))));m->words[01303]=(agc_word)(016000u^(rnd(&r)&077777u&rnd(&r)));
  m->words[0110]=(agc_word)(rnd(&r)&rnd(&r)&AGC_MASK);m->words[074]&=(agc_word)rnd(&r);m->words[0101]&=(agc_word)rnd(&r);
  m->words[0102]&=(agc_word)rnd(&r);m->words[0103]&=(agc_word)rnd(&r);}
 if(n%16u==5u)m->words[034]=0;
 if(n%16u==9u)m->words[034]=077777u;
 memset(io,0,sizeof *io);
 io->lm.superbank=(agc_word)((rnd(&r)%8u)*020u);io->lm.interrupt_enabled=0;io->hw.in_interrupt=1;
 io->hw.channel10=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel32=(agc_word)(rnd(&r)&AGC_MASK);
 io->hw.channel11=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel12=(agc_word)(rnd(&r)&AGC_MASK);
 io->hw.channel14=(agc_word)(rnd(&r)&AGC_MASK&~077000u);   /* no drive in progress (see LM-T4RUPT-IMU.md) */
 io->hw.channel30=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel33=(agc_word)(rnd(&r)&AGC_MASK);
 if(rnd(&r)%2u)io->hw.channel30=(agc_word)(m->words[01302]^(rnd(&r)&rnd(&r)&rnd(&r)&AGC_MASK));
 if(rnd(&r)%2u)io->hw.channel33=(agc_word)(m->words[01303]^(rnd(&r)&rnd(&r)&AGC_MASK));
}
unsigned lch_run(apollo_overlay_state *s,lch_io *io,uint16_t n,uint16_t *steps){
 static agc_routine_table tables[3];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=lm_t4rupt_core_routines;tables[1]=lm_t4rupt_imu_routines;tables[2]=lm_t4rupt_rr_routines;
 program.basic=tables;program.basic_count=3;program.hardware=&io->hw;
 lch_input(s,io,n);
 st=agc_transfer_run(&program,s,&io->lm,&io->lm.superbank,AGC_TRANSFER_NONE,LCH_MAX_STEPS,steps);
 return st==EXEC_UNMAPPED?LCH_RETURNED:st==EXEC_OUT_OF_SCOPE?LCH_REFUSED:3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lch_digest(apollo_overlay_state *s,uint16_t n){static lch_io io;unsigned i,kind;uint16_t steps;
 kind=lch_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LCH_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.lm.superbank);mix(io.lm.interrupt_enabled);mix(io.hw.in_interrupt);mix(io.hw.channel10);mix(io.hw.channel11);mix(io.hw.channel12);mix(io.hw.channel14);mix(io.hw.channel30);mix(io.hw.channel33);}
 return (uint32_t)sum_b<<16|sum_a;
}
