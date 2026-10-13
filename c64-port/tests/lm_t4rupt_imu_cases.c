/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_t4rupt_imu_cases.h"
static const agc_word entry_z[LM_T4RUPT_IMU_ENTRIES]={02172,02256,02263,02266,02313,02314,02321,02346,02363,02367,02373,02456,
 02462,02555,02611,02631,02642,02670,02675,02702,02703,02731};
agc_word lti_entry(uint16_t n){return n<LTI_IMUMON_CASES?entry_z[0]:entry_z[1u+(n-LTI_IMUMON_CASES)/LTI_PER_ENTRY];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
/* A bit-scan state the scan entries accept: RUPTREG1 the last index handled
 * (-1 before the first), RUPTREG2 the remaining changes, each selecting a
 * dispatch slot that is a TCF (IFAILJMP 0-3 and 5, C33JMP 0-2). */
static void scan(apollo_overlay_state *m,uint32_t *r,unsigned table){static const int b30[6]={-1,0,1,2,3,5},b33[3]={0,1,0};
 int base=table==30u?b30[rnd(r)%6u]:b33[rnd(r)%3u],i;agc_word d=0;
 for(i=base+1;i<=5;++i)if((table==30u?i!=4:i<=2)&&rnd(r)%2u)d|=(agc_word)(1u<<(14-(i-base)));
 m->words[070]=(agc_word)(base<0?077776u:base==0&&rnd(r)%2u?077777u:(unsigned)base);m->words[071]=d;
}
void lti_input(apollo_overlay_state *m,lti_io *io,uint16_t n){uint32_t r=0x06217200u+n*2654435761u;unsigned i,k;agc_word eb,z=lti_entry(n);
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054u;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 eb=(agc_word)(rnd(&r)%8u);m->words[3]=(agc_word)(eb<<8);m->words[4]=014000u;m->words[6]=(agc_word)(014000u|eb);m->words[5]=z;
 /* Q: a fixed-fixed return for SETISSW's callers. */
 m->words[2]=(agc_word)(06000u+rnd(&r)%0700u);
 /* IMODES30/33 near their flight values in half the cases: few changes per scan. */
 k=rnd(&r)%4u;
 if(k==0u){m->words[01302]=(agc_word)(037411u^(rnd(&r)&076777u&(rnd(&r)|rnd(&r))));m->words[01303]=(agc_word)(016000u^(rnd(&r)&077777u&rnd(&r)));}
 /* The signed-zero branches of CCS CDUZ (GLOCKMON) and CCS IMODES30 (TLIM). */
 if(n%16u==5u)m->words[034]=0;
 if(n%16u==9u)m->words[034]=077777u;
 if(z==02555u||z==02611u||z==02631u||z==02642u)scan(m,&r,30u);
 if(z==02670u||z==02675u||z==02702u)scan(m,&r,33u);
 if(z==02731u){k=rnd(&r)%4u;m->words[066]=k==0u?02225u:k==1u?02337u:k==2u?02656u:(agc_word)(06000u+rnd(&r)%0700u);
  if(k==0u)scan(m,&r,30u);
  if(k==2u)scan(m,&r,33u);}
 memset(io,0,sizeof *io);
 io->lm.superbank=(agc_word)((rnd(&r)%8u)*020u);io->lm.interrupt_enabled=0;io->hw.in_interrupt=1;
 io->hw.channel11=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel12=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel14=(agc_word)(rnd(&r)&AGC_MASK);
 if(z==02611u)io->hw.channel14&=(agc_word)~077000u;  /* IMUCAGE cleared the drive and gyro bits */
 io->hw.channel30=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel33=(agc_word)(rnd(&r)&AGC_MASK);
 /* Channel 30 near IMODES30 too, so only some bits change. */
 if(rnd(&r)%2u)io->hw.channel30=(agc_word)(m->words[01302]^(rnd(&r)&rnd(&r)&rnd(&r)&AGC_MASK));
 if(rnd(&r)%2u)io->hw.channel33=(agc_word)(m->words[01303]^(rnd(&r)&rnd(&r)&AGC_MASK));
 /* TLIM with the updated IMODES30 +0: only 30RDMSK bits, temperature bit 15 dropping. */
 if(z==02172u&&n%16u==13u){m->words[01302]=(agc_word)((rnd(&r)&076400u)|040000u);io->hw.channel30&=(agc_word)~076400u;}
 /* TLIM with the updated IMODES30 -0: every other bit set, temperature bit 15 rising. */
 if(z==02172u&&n%16u==14u){m->words[01302]=037777u;io->hw.channel30=077777u;}
}
unsigned lti_run(apollo_overlay_state *s,lti_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&lm_t4rupt_imu_routines;program.basic_count=1;program.hardware=&io->hw;
 lti_input(s,io,n);
 call=agc_transfer_resolve(&program,s,io->lm.superbank);
 if(!call)return 3u;
 return call(&program,s,&io->lm)==EXEC_OK?LTI_RETURNED:LTI_REFUSED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lti_digest(apollo_overlay_state *s,uint16_t n){static lti_io io;unsigned i,kind=lti_run(s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LTI_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.lm.superbank);mix(io.hw.channel11);mix(io.hw.channel12);mix(io.hw.channel14);mix(io.hw.channel30);mix(io.hw.channel33);}
 return (uint32_t)sum_b<<16|sum_a;
}
