/* SPDX-License-Identifier: MIT */
#include "lm_fresh_start_cases.h"
#include "lm_interbank_routines.h"
#include "../src/lm_fresh_start_operands.inc"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
void lm_fs_input(apollo_overlay_state *s,lm_fresh_io *io,agc_hardware *hw,uint16_t n){
 unsigned i,k=n%LM_FS_ROUTINES,eb,g;uint32_t seed=UINT32_C(0x2447)+(uint32_t)n*UINT32_C(7919);agc_word fb,z=lm_fresh_entries[k];
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
 /* Restart groups 1-6: PHASEn and -PHASEn agree, n inactive (+0), active or negative. */
 for(g=0;g<6u;++g){unsigned m=upto(&seed,4);agc_word ph=m==0u?0u:m==1u?(agc_word)(1u+upto(&seed,040)):m==2u?word(&seed):(agc_word)(AGC_MASK-upto(&seed,040));
  s->words[0753u+2u*g]=ph;s->words[0752u+2u*g]=(agc_word)(~ph&AGC_MASK);}
 /* An eighth corrupt: a bit of one -PHASEn flipped, or (a quarter of
  * those) -PHASEn equal to PHASEn, which PCLOOP's RXOR makes +0. */
 if(upto(&seed,8)==0u){g=upto(&seed,6);
  if(upto(&seed,4))s->words[0752u+2u*g]^=(agc_word)(1u<<upto(&seed,15));else s->words[0752u+2u*g]=s->words[0753u+2u*g];}
 /* The group scan's counters, as PINACT finds them after RESTARTS. */
 s->words[0161]=(agc_word)upto(&seed,6);s->words[0162]=(agc_word)upto(&seed,7);
 /* ERESTORE: +0, -0, the address under test (SKEEP7 equal) or corrupt. */
 {unsigned m=upto(&seed,4);agc_word a=(agc_word)(061u+upto(&seed,01776u-061u+1u));
  if(m==0u)s->words[LM_FS_ERESTORE]=0;else if(m==1u)s->words[LM_FS_ERESTORE]=AGC_MASK;
  else if(m==2u)s->words[LM_FS_ERESTORE]=s->words[LM_FS_SKEEP7]=a;}
 s->words[LM_FS_SKEEP4]=(agc_word)(upto(&seed,8)<<8);
 eb=upto(&seed,8);
 if(z>=04000u){fb=(agc_word)((4u+upto(&seed,032u))<<10);io->control.superbank=(agc_word)(upto(&seed,2)?0060u:0u);}
 else{fb=(agc_word)(05u<<10);io->control.superbank=(agc_word)(upto(&seed,4)<<4);}
 /* A BANKCALL'd KLEAN entry returns to SWRETURN or another fixed-fixed word. */
 if(z==02643u||z==02647u||z==02652u)s->words[2]=upto(&seed,2)?04631u:(agc_word)(07000u+upto(&seed,01000u));
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[5]=z;
}
enum exec_status lm_fs_run(apollo_overlay_state *s,lm_fresh_io *io,agc_hardware *hw,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,lm_fresh_operands,LM_FRESH_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=lm_fresh_start_routines;tables[1]=lm_interbank_routines;
 program.basic=tables;program.basic_count=2;program.hardware=hw;
 lm_fs_input(s,io,hw,n);
 /* It ends at the first point no table owns, or at SWRETURN. */
 st=agc_transfer_run(&program,s,io,&io->control.superbank,LM_FS_SWRETURN_POINT,24,steps);
 return st==EXEC_UNMAPPED||st==EXEC_OK?EXEC_OK:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_fs_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){lm_fresh_io io;agc_hardware hw;uint16_t steps=0;unsigned i;
 *status=lm_fs_run(s,&io,&hw,n,&steps);sum_a=sum_b=0;
 mix(io.control.interrupt_enabled);mix(io.control.superbank);mix(hw.channel11);mix(io.channel5);mix(io.channel6);
 mix(io.channel12);mix(io.channel13);mix(io.channel14);mix(io.trap31a);mix(io.trap31b);mix(io.trap32);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
