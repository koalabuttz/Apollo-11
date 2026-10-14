/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_t4rupt_chain_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
void cch_input(apollo_overlay_state *m,cch_io *io,uint16_t n){uint32_t r=0x0c4a1c40u+n*2654435761u;unsigned i,k;agc_word eb,x;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054u;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 /* T4RUPT as its vector enters it: FB 06, EB 6 (T4RPTBB) in most cases. */
 x=(agc_word)(rnd(&r)%4u);eb=(agc_word)(x?6u:rnd(&r)%8u);
 m->words[3]=(agc_word)(eb<<8);m->words[4]=014000u;m->words[6]=(agc_word)(014000u|eb);m->words[5]=02000u;
 /* DSRUPTSW: a phase count (each T4JUMP target), +0, or the quick display. */
 x=(agc_word)(rnd(&r)%8u);
 if(x<5u)m->words[01302]=(agc_word)(rnd(&r)%9u);else if(x==5u)m->words[01302]=0;else m->words[01302]=(agc_word)((rnd(&r)&037777u)|040000u);
 if(x==7u)m->words[01302]|=0377u;
 m->words[0776]=(agc_word)(rnd(&r)%12u);if(m->words[0776]==013u)m->words[0776]=077777u;
 for(i=0;i<12u;++i){x=(agc_word)(rnd(&r)&AGC_MASK);if(x==077777u||rnd(&r)%4u)x&=037777u;if(x==0u&&i<11u)x=1u;m->words[01023+i]=x;}
 if(rnd(&r)%8u==0u)m->words[01036]=0;
 x=(agc_word)(rnd(&r)%4u);
 if(x==0u)m->words[01016]=0;else if(x==1u)m->words[01016]=(agc_word)(rnd(&r)%013u);else m->words[01016]=(agc_word)(rnd(&r)&AGC_MASK);
 /* IMODES30/33 and the flag words near flight values in half the cases. */
 k=rnd(&r)%2u;
 if(k){x=(agc_word)(rnd(&r)|rnd(&r));m->words[01320]=(agc_word)(037411u^(rnd(&r)&076777u&x));
  x=(agc_word)rnd(&r);m->words[01321]=(agc_word)(016000u^(rnd(&r)&077777u&x));
  for(i=074;i<=0104u;++i){x=(agc_word)rnd(&r);m->words[i]&=x;}}
 if(n%16u==5u)m->words[034]=0;
 if(n%16u==9u)m->words[034]=077777u;
 memset(io,0,sizeof *io);
 io->cm.io.superbank=(agc_word)((rnd(&r)%8u)*020u);io->cm.io.interrupt_enabled=0;io->hw.in_interrupt=1;
 io->hw.channel10=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel32=(agc_word)(rnd(&r)&AGC_MASK);
 io->hw.channel11=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel12=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel13=(agc_word)(rnd(&r)&AGC_MASK&~070000u);
 io->hw.channel14=(agc_word)(rnd(&r)&AGC_MASK&~077000u);   /* no drive in progress (see CM-T4RUPT-IMU.md) */
 io->hw.channel30=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel31=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel33=(agc_word)(rnd(&r)&AGC_MASK);
 if(rnd(&r)%2u){x=(agc_word)(rnd(&r)&rnd(&r));io->hw.channel30=(agc_word)(m->words[01320]^(rnd(&r)&x&AGC_MASK));}
 if(rnd(&r)%2u){x=(agc_word)rnd(&r);io->hw.channel33=(agc_word)(m->words[01321]^(rnd(&r)&x&AGC_MASK));}
 io->in=io->hw;
 io->cm.io.channel11=io->hw.channel11;io->cm.io.channel12=io->hw.channel12;io->cm.io.channel13=io->hw.channel13;io->cm.io.channel14=io->hw.channel14;
 io->cm.in.channel30=io->hw.channel30;io->cm.in.channel31=io->hw.channel31;io->cm.in.channel32=io->hw.channel32;io->cm.in.channel33=io->hw.channel33;
}
static agc_word *hw_channel(agc_hardware *h,unsigned k){
 switch(k){case 010:return &h->channel10;case 011:return &h->channel11;case 012:return &h->channel12;case 013:return &h->channel13;case 014:return &h->channel14;
  case 030:return &h->channel30;case 031:return &h->channel31;case 032:return &h->channel32;default:return &h->channel33;}}
static agc_word *optics_channel(cch_io *io,unsigned k){
 switch(k){case 011:return &io->cm.io.channel11;case 012:return &io->cm.io.channel12;case 013:return &io->cm.io.channel13;case 014:return &io->cm.io.channel14;
  case 030:return &cm_channels.channel30;case 031:return &cm_channels.channel31;case 032:return &cm_channels.channel32;case 033:return &cm_channels.channel33;default:return 0;}}
static const unsigned channels[9]={010,011,012,013,014,030,031,032,033};
agc_word cch_channel(const cch_io *io,unsigned k){cch_io *v=(cch_io *)io;agc_word in=*hw_channel(&v->in,k),*o=optics_channel(v,k);
 if(*hw_channel(&v->hw,k)!=in||!o)return *hw_channel(&v->hw,k);
 return *o;}
unsigned cch_run(apollo_overlay_state *s,cch_io *io,uint16_t n,uint16_t *steps){
 static agc_routine_table tables[3];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;unsigned i;
 tables[0]=cm_t4rupt_core_routines;tables[1]=cm_t4rupt_imu_routines;tables[2]=cm_t4rupt_optics_basic;
 program.basic=tables;program.basic_count=3;program.hardware=&io->hw;
 cch_input(s,io,n);
 cm_channels.channel05=cm_channels.channel06=0;
 cm_channels.channel30=io->hw.channel30;cm_channels.channel31=io->hw.channel31;cm_channels.channel32=io->hw.channel32;cm_channels.channel33=io->hw.channel33;
 st=agc_transfer_run(&program,s,&io->cm,&io->cm.io.superbank,AGC_TRANSFER_NONE,CCH_MAX_STEPS,steps);
 for(i=0;i<9u;++i){agc_word in=*hw_channel(&io->in,channels[i]),*o=optics_channel(io,channels[i]);
  if(o&&*hw_channel(&io->hw,channels[i])!=in&&*o!=in)return 4u;}
 return st==EXEC_UNMAPPED?CCH_RETURNED:st==EXEC_OUT_OF_SCOPE?CCH_REFUSED:3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cch_digest(apollo_overlay_state *s,uint16_t n){static cch_io io;unsigned i,kind;uint16_t steps;
 kind=cch_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=CCH_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.cm.io.superbank);mix(io.cm.io.interrupt_enabled);mix(io.hw.in_interrupt);
  for(i=0;i<9u;++i)mix(cch_channel(&io,channels[i]));}
 return (uint32_t)sum_b<<16|sum_a;
}
