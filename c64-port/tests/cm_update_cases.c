/* SPDX-License-Identifier: MIT */
#include "cm_update_cases.h"
#include "cm_interbank_routines.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
void cm_up_input(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,uint16_t n){
 unsigned i,k=n%CM_UP_ROUTINES,eb,count;uint32_t seed=UINT32_C(0x3373)+(uint32_t)n*UINT32_C(6007);agc_word fb,verb,z=cm_up_entries[k].z;
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=023;++i)s->words[i]=0;for(i=026;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {cm_native_io zero={0};*io=zero;}
 {agc_hardware zero={0};*hw=zero;}
 io->interrupt_enabled=(agc_word)(upto(&seed,2));hw->channel11=word(&seed);
 verb=(agc_word)upto(&seed,4);s->words[CM_UP_UPVERB]=verb;
 /* COMPNUMB as OHWELL1 accepts it: 2 for V70/V73, 3-20 for V71, odd 3-19 for V72. */
 count=verb==1u?3u+upto(&seed,18):verb==2u?3u+2u*upto(&seed,9):2u;s->words[CM_UP_COMPNUMB]=(agc_word)count;
 s->words[CM_UP_UPCOUNT]=(agc_word)(1u+upto(&seed,count));
 {unsigned t=upto(&seed,61);s->words[CM_UP_UPTEMP]=t>=30u?(agc_word)(t-30u):(agc_word)(~(30u-t)&AGC_MASK);}
 if(z==03613u||z==03621u)s->words[CM_UP_UPTEMP]=upto(&seed,2)?03534u:03721u;
 /* The component number V71/V72 asked for (UPBUFF) and the data words. */
 {unsigned t=upto(&seed,36);s->words[CM_UP_UPBUFF]=t>=5u?(agc_word)(t-5u):(agc_word)(~(5u-t)&AGC_MASK);}
 if(z!=03424u&&upto(&seed,2))s->words[CM_UP_UPBUFF]=word(&seed);
 /* V71: UPBUFF+1 is the first address (EBANK bits and S); V72: every other word. */
 if(verb==1u)s->words[CM_UP_UPBUFF+1u]=(agc_word)(((1u+upto(&seed,7))<<8)|upto(&seed,0400));
 if(verb==2u)for(i=1;i<=21u;i+=2u)s->words[CM_UP_UPBUFF+i]=(agc_word)(((1u+upto(&seed,7))<<8)|upto(&seed,0400));
 /* MODREG +0, -0, 2 (the one mode CKMDMORE takes) or a mode; FLAGWRD5 bit 8 (CKMDMORE) often set. */
 {unsigned m=upto(&seed,5);if(m==0u)s->words[01011]=0;else if(m==1u)s->words[01011]=077777u;else if(m==2u)s->words[01011]=2u;else s->words[01011]=(agc_word)upto(&seed,0100);}
 if(upto(&seed,2))s->words[0101]|=0200u;
 eb=upto(&seed,8);
 if(cm_up_entries[k].bank==043u){fb=(agc_word)(033u<<10);io->superbank=0100;}
 else{fb=(agc_word)(027u<<10);io->superbank=(agc_word)(upto(&seed,2)?0060u:0u);}
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[5]=z;
}
enum exec_status cm_up_run(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=cm_update_routines;tables[1]=cm_interbank_routines;
 program.basic=tables;program.operands=cm_interbank_site_table(&program.operand_count);program.basic_count=2;program.hardware=hw;
 cm_up_input(s,io,hw,n);
 /* No stop inside the run: it ends at the first point no table owns. */
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,24,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st==EXEC_OK?EXEC_OUT_OF_SCOPE:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_up_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){cm_native_io io;agc_hardware hw;uint16_t steps=0;unsigned i;
 *status=cm_up_run(s,&io,&hw,n,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(io.superbank);mix(hw.channel11);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
