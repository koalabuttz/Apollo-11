/* SPDX-License-Identifier: MIT */
#include "lm_v37_cases.h"
#include "lm_interbank_routines.h"
#include "../src/lm_v37_operands.inc"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
/* A major mode: one PREMM1 lists, P00, P70/P71, or any of 0-0177. */
static agc_word mode(uint32_t *seed){unsigned m=upto(seed,8);
 if(m<4u)return (agc_word)(lm_v37_premm1[upto(seed,30)]&0177u);
 if(m==4u)return 0u;
 if(m==5u)return (agc_word)(0106u+upto(seed,2));
 return (agc_word)upto(seed,0200u);}
void lm_v37_input(apollo_overlay_state *s,lm_fresh_io *io,agc_hardware *hw,uint16_t n){
 unsigned i,k=n%LM_V37_ROUTINES,eb,g;uint32_t seed=UINT32_C(0x2040)+(uint32_t)n*UINT32_C(7919);agc_word z=lm_v37_entries[k];
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=023;++i)s->words[i]=0;for(i=026;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {lm_fresh_io zero={{0,0},0,0,0,0,0,0,0,0,0,0,0};*io=zero;}
 {agc_hardware zero={0};*hw=zero;}
 io->control.interrupt_enabled=(uint16_t)upto(&seed,2);hw->channel11=word(&seed);
 io->channel5=word(&seed);io->channel6=word(&seed);io->channel12=word(&seed);
 io->channel13=(agc_word)(word(&seed)&02767u);io->channel14=(agc_word)(word(&seed)&0777u);
 io->channel15=upto(&seed,4)?(agc_word)upto(&seed,040):022u;
 io->channel16=word(&seed);io->channel33=word(&seed);
 io->trap31a=(uint16_t)upto(&seed,2);io->trap31b=(uint16_t)upto(&seed,2);io->trap32=(uint16_t)upto(&seed,2);
 for(g=0;g<6u;++g){unsigned m=upto(&seed,4);agc_word ph=m==0u?0u:m==1u?(agc_word)(1u+upto(&seed,040)):m==2u?word(&seed):(agc_word)(AGC_MASK-upto(&seed,040));
  s->words[0753u+2u*g]=ph;s->words[0752u+2u*g]=(agc_word)(~ph&AGC_MASK);}
 s->words[0161]=(agc_word)upto(&seed,6);s->words[0162]=(agc_word)upto(&seed,7);
 s->words[LM_FS_ERESTORE]=upto(&seed,2)?0u:AGC_MASK;
 s->words[LM_FS_SKEEP4]=(agc_word)(upto(&seed,8)<<8);
 /* V37's request (A), MINDEX, MODREG. */
 s->words[LM_V37_MMNUMBER]=mode(&seed);
 if(z==02040u){s->words[0]=upto(&seed,4)?mode(&seed):(agc_word)(0106u+upto(&seed,2));
  /* Mostly an IMU not being initialized and NODOFLAG clear, so V37 gets past CANTROD. */
  if(upto(&seed,8))s->words[01302]&=(agc_word)~LM_V37_K_BIT6;
  if(upto(&seed,8))s->words[076]&=(agc_word)~LM_V37_K_NODOBIT;}
 s->words[LM_V37_MINDEX]=(agc_word)upto(&seed,30);
 {unsigned m=upto(&seed,4);static const agc_word near[6]={024u,026u,031u,023u,030u,0u};
  if(m<2u)s->words[01011]=near[upto(&seed,6)];else if(m==2u)s->words[01011]=mode(&seed);}
 eb=upto(&seed,8);io->control.superbank=(agc_word)(upto(&seed,4)<<4);
 s->words[4]=(agc_word)(04u<<10);s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(s->words[4]|eb);s->words[5]=z;
}
/* lm_interbank_routines but ISWRETRN: a KLEAN entry under IBNKCALL returns
 * there with Q = 3, which lm_interbank does not take, so the run ends. */
static agc_routine_entry interbank[16];
static agc_routine_table interbank_table(void){unsigned i;uint16_t n=0;
 for(i=0;i<lm_interbank_routines.count;++i)if(lm_interbank_routines.entries[i].physical!=LM_V37_ISWRETRN_POINT)interbank[n++]=lm_interbank_routines.entries[i];
 {agc_routine_table t;t.entries=interbank;t.count=n;return t;}}
enum exec_status lm_v37_run(apollo_overlay_state *s,lm_fresh_io *io,agc_hardware *hw,uint16_t n,uint16_t *steps){
 agc_routine_table tables[3];agc_native_program program={0,0,0,0,lm_v37_operands,LM_V37_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=lm_v37_routines;tables[1]=lm_fresh_start_routines;tables[2]=interbank_table();
 program.basic=tables;program.basic_count=3;program.hardware=hw;
 lm_v37_input(s,io,hw,n);
 st=agc_transfer_run(&program,s,io,&io->control.superbank,LM_V37_SWRETURN_POINT,32,steps);
 return st==EXEC_UNMAPPED||st==EXEC_OK?EXEC_OK:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_v37_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){lm_fresh_io io;agc_hardware hw;uint16_t steps=0;unsigned i;
 *status=lm_v37_run(s,&io,&hw,n,&steps);sum_a=sum_b=0;
 mix(io.control.interrupt_enabled);mix(io.control.superbank);mix(hw.channel11);mix(io.channel5);mix(io.channel6);
 mix(io.channel12);mix(io.channel13);mix(io.channel14);mix(io.trap31a);mix(io.trap31b);mix(io.trap32);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
