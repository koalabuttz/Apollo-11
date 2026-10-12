/* SPDX-License-Identifier: MIT */
#include "lm_update_cases.h"
#include "lm_interbank_routines.h"
#include "../src/lm_update_operands.inc"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
/* An update address word (EBANK bits and S) for span words from S on: any
 * erasable bank; in EB0 from 061 up (below are the registers and counters,
 * which the native code refuses), clear of the words the update itself runs
 * on (MPAC). */
static agc_word target(uint32_t *seed,unsigned span){
 for(;;){unsigned eb=upto(seed,8),s=upto(seed,0400);
  if(eb==0u&&(s<061u||(s+span>0154u&&s<=0156u)))continue;
  return (agc_word)(eb<<8|s);}
}
void lm_up_input(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,uint16_t n){
 unsigned i,k=n%LM_UP_ROUTINES,eb,count;uint32_t seed=UINT32_C(0x3401)+(uint32_t)n*UINT32_C(6007);agc_word fb,verb,z=lm_up_entries[k].z;
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=023;++i)s->words[i]=0;for(i=026;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {p66_bias_io zero={0};*io=zero;}
 {agc_hardware zero={0};*hw=zero;}
 io->interrupt_enabled=(agc_word)(upto(&seed,2));hw->channel11=word(&seed);
 verb=(agc_word)upto(&seed,4);s->words[LM_UP_UPVERB]=verb;
 /* COMPNUMB as OHWELL1 accepts it: 2 for V70/V73, 3-20 for V71 and V72
  * (UPEND72 rejects an even count). */
 count=verb==1u||verb==2u?3u+upto(&seed,18):2u;s->words[LM_UP_COMPNUMB]=(agc_word)count;
 s->words[LM_UP_UPCOUNT]=(agc_word)(1u+upto(&seed,count));
 {unsigned t=upto(&seed,61);s->words[LM_UP_UPTEMP]=t>=30u?(agc_word)(t-30u):(agc_word)(~(30u-t)&AGC_MASK);}
 if(z==03621u||z==03627u)s->words[LM_UP_UPTEMP]=upto(&seed,2)?03542u:03727u;
 /* The component number V71/V72 asked for (UPBUFF) and the data words. */
 {unsigned t=upto(&seed,36);s->words[LM_UP_UPBUFF]=t>=5u?(agc_word)(t-5u):(agc_word)(~(5u-t)&AGC_MASK);}
 /* After the II load (TC CK4V32 returning to 03432) UPBUFF is the II: below 3, 3-20 or above 20. */
 if(z!=03431u&&upto(&seed,2))s->words[LM_UP_UPBUFF]=word(&seed);
 /* V71: UPBUFF+1 is the first address (EBANK bits and S); V72: every other word.
  * One V71 in four is a one-word load at the first word of a bank (UPEND71's
  * BZF STORLP71) and one in four starts near the end of a bank (some run past
  * it: UPEND71's EBANK-overflow error). */
 if(verb==1u){unsigned m=upto(&seed,4);
  if(m==0u){count=3u;s->words[LM_UP_COMPNUMB]=3u;s->words[LM_UP_UPCOUNT]=(agc_word)(1u+upto(&seed,3));s->words[LM_UP_UPBUFF+1u]=(agc_word)((1u+upto(&seed,7))<<8);}
  else if(m==1u)s->words[LM_UP_UPBUFF+1u]=(agc_word)(((1u+upto(&seed,7))<<8)|(0377u-upto(&seed,count)));
  else s->words[LM_UP_UPBUFF+1u]=target(&seed,count-2u);}
 if(verb==2u)for(i=1;i<=21u;i+=2u)s->words[LM_UP_UPBUFF+i]=target(&seed,1u);
 {unsigned m=upto(&seed,4);if(m==0u)s->words[01011]=0;else if(m==1u)s->words[01011]=077777u;else s->words[01011]=(agc_word)upto(&seed,0100);}
 eb=upto(&seed,8);
 if(lm_up_entries[k].bank==043u){fb=(agc_word)(033u<<10);io->superbank=0100;}
 else{fb=(agc_word)(04u<<10);io->superbank=(agc_word)(upto(&seed,2)?0060u:0u);}
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[5]=z;
}
enum exec_status lm_up_run(apollo_overlay_state *s,p66_bias_io *io,agc_hardware *hw,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,lm_update_operands,LM_UPDATE_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=lm_update_routines;tables[1]=lm_interbank_routines;
 program.basic=tables;program.basic_count=2;program.hardware=hw;
 lm_up_input(s,io,hw,n);
 /* No stop inside the run: it ends at the first point no table owns. */
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,24,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st==EXEC_OK?EXEC_OUT_OF_SCOPE:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_up_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){p66_bias_io io;agc_hardware hw;uint16_t steps=0;unsigned i;
 *status=lm_up_run(s,&io,&hw,n,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(io.superbank);mix(hw.channel11);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
