/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "t4rupt_core_cases.h"
static const agc_word lm_ret[3]={02116,02121,02125};
static const agc_word cm_ret[2]={02116,02130};
unsigned t4_cases(unsigned mission){return mission?T4_CM_CASES:T4_LM_CASES;}
agc_word t4_entry(unsigned mission,uint16_t n){return n<T4_T4RUPT_CASES?02000u:(mission?cm_ret:lm_ret)[(n-T4_T4RUPT_CASES)/T4_RETURN_CASES];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
void t4_input(unsigned mission,apollo_overlay_state *m,t4_io *io,uint16_t n){uint32_t r=0x04040404u+mission*0x9e3779b9u+n*2654435761u;unsigned i;agc_word eb,x,dsruptsw=mission?01302u:01313u;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=0;m->words[035]=0;m->words[036]=0;m->words[047]=0;for(i=050;i<=054;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 eb=(agc_word)(rnd(&r)%8u);m->words[3]=(agc_word)(eb<<8);m->words[4]=014000u;m->words[6]=(agc_word)(014000u|eb);m->words[5]=t4_entry(mission,n);
 /* DSRUPTSW: a phase count, or negative (the 20 ms quick display). */
 x=(agc_word)(rnd(&r)%8u);m->words[dsruptsw]=(agc_word)(x<3u?rnd(&r)%9u:x==3u?0u:(rnd(&r)&037777u)|040000u);
 /* Some that SYNCT4's BIT9 steps carry to -0. */
 if(x==7u)m->words[dsruptsw]|=0377u;
 m->words[0776]=(agc_word)(rnd(&r)%12u);if(m->words[0776]==013u)m->words[0776]=077777u;
 /* DSPTAB: mostly unchanged (positive), some waiting (negative). */
 for(i=0;i<12u;++i){x=(agc_word)(rnd(&r)&AGC_MASK);if(x==077777u||rnd(&r)%4u)x&=037777u;if(x==0u&&i<11u)x=1u;m->words[01023+i]=x;}
 if(rnd(&r)%8u==0u)m->words[01036]=0;   /* the lamps word (DSPTAB +11D) +0 */
 x=(agc_word)(rnd(&r)%4u);m->words[01016]=(agc_word)(x==0u?0u:x==1u?rnd(&r)%013u:rnd(&r)&AGC_MASK);
 m->words[070]=(agc_word)(rnd(&r)%8u);
 /* FLAGWRD5 -0 (DSPOUT's CCS FLAGWRD5 takes its fourth branch); past the
    first 128 cases, which the resident interrupt vectors share. */
 if(n>=128u&&n%16u==15u)m->words[0101]=077777u;
 memset(io,0,sizeof *io);
 if(mission)io->cm.superbank=(agc_word)((rnd(&r)%8u)*020u);else io->lm.superbank=(agc_word)((rnd(&r)%8u)*020u);
 io->hw.in_interrupt=1;io->hw.channel10=(agc_word)(rnd(&r)&AGC_MASK);io->hw.channel32=(agc_word)(rnd(&r)&AGC_MASK);
}
unsigned t4_run(unsigned mission,apollo_overlay_state *s,t4_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=mission?&cm_t4rupt_core_routines:&lm_t4rupt_core_routines;program.basic_count=1;program.hardware=&io->hw;
 t4_input(mission,s,io,n);
 call=agc_transfer_resolve(&program,s,mission?io->cm.superbank:io->lm.superbank);
 if(!call)return 3u;
 return call(&program,s,mission?(void *)&io->cm:(void *)&io->lm)==EXEC_OK?T4_RETURNED:T4_REFUSED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t t4_digest(unsigned mission,apollo_overlay_state *s,uint16_t n){static t4_io io;unsigned i,kind=t4_run(mission,s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=T4_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(mission?io.cm.superbank:io.lm.superbank);mix(mission?io.cm.interrupt_enabled:io.lm.interrupt_enabled);mix(io.hw.in_interrupt);mix(io.hw.channel10);}
 return (uint32_t)sum_b<<16|sum_a;
}
