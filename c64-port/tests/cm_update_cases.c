/* SPDX-License-Identifier: MIT */
#include "cm_update_cases.h"
#include "cm_interbank_routines.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
/* An update address word (EBANK bits and S) for span words from S on: any
 * erasable bank; in EB0 from 061 up (below are the registers and counters,
 * which the native code refuses), clear of the words the update itself runs
 * on (MPAC, COMPNUMB-UPTEMP). */
static agc_word target(uint32_t *seed,unsigned span){
 for(;;){unsigned eb=upto(seed,8),s=upto(seed,0400);
  if(eb==0u&&(s<061u||(s+span>0154u&&s<=0156u)||(s>=0300u-span&&s<=0330u)))continue;
  return (agc_word)(eb<<8|s);}
}
void cm_up_input(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,uint16_t n){
 unsigned i,k=n%(CM_UP_ROUTINES+CM_UP_INTERPRETIVE),eb,count;uint32_t seed=UINT32_C(0x3373)+(uint32_t)n*UINT32_C(6007);agc_word fb,verb,z=k<CM_UP_ROUTINES?cm_up_entries[k].z:0u;
 for(i=0;i<2048u;++i)s->words[i]=word(&seed);
 s->words[7]=0;for(i=010;i<=023;++i)s->words[i]=0;for(i=026;i<=060;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 {cm_native_io zero={0};*io=zero;}
 {agc_hardware zero={0};*hw=zero;}
 io->interrupt_enabled=(agc_word)(upto(&seed,2));hw->channel11=word(&seed);
 verb=(agc_word)upto(&seed,4);s->words[CM_UP_UPVERB]=verb;
 /* COMPNUMB as OHWELL1 accepts it: 2 for V70/V73, 3-20 for V71 and V72
  * (UPEND72 rejects an even count). */
 count=verb==1u||verb==2u?3u+upto(&seed,18):2u;s->words[CM_UP_COMPNUMB]=(agc_word)count;
 s->words[CM_UP_UPCOUNT]=(agc_word)(1u+upto(&seed,count));
 {unsigned t=upto(&seed,61);s->words[CM_UP_UPTEMP]=t>=30u?(agc_word)(t-30u):(agc_word)(~(30u-t)&AGC_MASK);}
 if(z==03613u||z==03621u)s->words[CM_UP_UPTEMP]=upto(&seed,2)?03534u:03721u;
 /* The component number V71/V72 asked for (UPBUFF) and the data words. */
 {unsigned t=upto(&seed,36);s->words[CM_UP_UPBUFF]=t>=5u?(agc_word)(t-5u):(agc_word)(~(5u-t)&AGC_MASK);}
 /* After the II load (TC CK4V32 returning to 03424) UPBUFF is the II: below 3, 3-20 or above 20. */
 if(z!=03423u&&upto(&seed,2))s->words[CM_UP_UPBUFF]=word(&seed);
 /* V71: UPBUFF+1 is the first address (EBANK bits and S); V72: every other word.
  * One V71 in four is a one-word load at the first word of a bank (UPEND71's
  * BZF STORLP71) and one in four starts near the end of a bank (some run past
  * it: UPEND71's EBANK-overflow error). */
 if(verb==1u){unsigned m=upto(&seed,4);
  if(m==0u){count=3u;s->words[CM_UP_COMPNUMB]=3u;s->words[CM_UP_UPCOUNT]=(agc_word)(1u+upto(&seed,3));s->words[CM_UP_UPBUFF+1u]=(agc_word)((1u+upto(&seed,7))<<8);}
  else if(m==1u)s->words[CM_UP_UPBUFF+1u]=(agc_word)(((1u+upto(&seed,7))<<8)|(0377u-upto(&seed,count)));
  else s->words[CM_UP_UPBUFF+1u]=target(&seed,count-2u);}
 if(verb==2u)for(i=1;i<=21u;i+=2u)s->words[CM_UP_UPBUFF+i]=target(&seed,1u);
 /* MODREG +0, -0, 2 (the one mode CKMDMORE takes) or a mode; FLAGWRD5 bit 8 (CKMDMORE) often set. */
 {unsigned m=upto(&seed,5);if(m==0u)s->words[01011]=0;else if(m==1u)s->words[01011]=077777u;else if(m==2u)s->words[01011]=2u;else s->words[01011]=(agc_word)upto(&seed,0100);}
 if(upto(&seed,2))s->words[0101]|=0200u;
 eb=upto(&seed,8);
 if(k<CM_UP_ROUTINES&&cm_up_entries[k].bank==043u){fb=(agc_word)(033u<<10);io->superbank=0100;}
 else{fb=(agc_word)(027u<<10);io->superbank=(agc_word)(upto(&seed,2)?0060u:0u);}
 /* UPACTOFF is entered as its two callers outside the file reach it: TC BANKCALL
  * with Q at the CADR after it (FRESH_START 04,2152; P20-P25 34,3400). */
 if(z==03757u){if((n/(CM_UP_ROUTINES+CM_UP_INTERPRETIVE))&1u){fb=(agc_word)(04u<<10);s->words[2]=02152u;}else{fb=(agc_word)(034u<<10);io->superbank=0060u;s->words[2]=03400u;}z=CM_UP_BANKCALL;}
 s->words[4]=fb;s->words[3]=(agc_word)(eb<<8);s->words[6]=(agc_word)(fb|eb);s->words[5]=z;
 /* UPJOB's interpretive words: NEWOPS with LOC on the word, BANKSET = BBANK
  * (FB 27), EDOP +0, a work area of one of the five VAC areas with PUSHLOC
  * inside it, and NEWJOB +0 or, in a fifth of the cases, positive. */
 if(k>=CM_UP_ROUTINES){agc_word vac;
  s->words[5]=CM_UP_NEWOPS;s->words[CM_UP_LOC]=cm_up_interp_entries[k-CM_UP_ROUTINES];s->words[CM_UP_BANKSET]=s->words[6];s->words[023]=0;
  s->words[067]=upto(&seed,5)==4u?(agc_word)(upto(&seed,0400)+1u):0u;
  vac=(agc_word)(0401u+054u*upto(&seed,5));s->words[0120]=vac;s->words[0166]=(agc_word)(vac+upto(&seed,043));}
}
/* The original's interpreter (fixed-fixed 06006-07656, bank 0 00,2000-3731
 * and bank 1 01,2371-2625) and NEWOPS with LOC on one of UPJOB's
 * interpretive words: the points an interpretive case's run owns besides
 * the file's own. */
int cm_up_interpreter(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset){
 uint16_t ph;
 if(z==CM_UP_NEWOPS){ph=agc_fixed_physical(loc,bankset,superbank);return ph==027u*1024u+01544u||ph==027u*1024u+01546u;}
 if(z>=06006u&&z<=07656u)return 1;
 if(z<02000u||z>=04000u)return 0;
 ph=agc_fixed_physical(z,fb,superbank);
 return ph<=03731u-02000u||(ph>=1024u+0371u&&ph<=1024u+0625u);
}
enum exec_status cm_up_run(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 tables[0]=cm_update_routines;tables[1]=cm_interbank_routines;
 program.basic=tables;program.operands=cm_interbank_site_table(&program.operand_count);program.basic_count=2;program.hardware=hw;
 program.interpretive=&cm_update_interpretive;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+CM_UP_NEWOPS-04000u);program.loc=CM_UP_LOC;program.bankset=CM_UP_BANKSET;
 cm_up_input(s,io,hw,n);
 /* The run ends at the first point no table owns, or at SWRETURN, where
  * UPACTOFF's TC Q leaves a BANKCALL with Q = 3 (cm_interbank refuses it). */
 st=agc_transfer_run(&program,s,io,&io->superbank,CM_UP_SWRETURN_POINT,24,steps);
 return st==EXEC_UNMAPPED||(st==EXEC_OK&&s->words[5]==CM_UP_SWRETURN)?EXEC_OK:st==EXEC_OK?EXEC_OUT_OF_SCOPE:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_up_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){cm_native_io io;agc_hardware hw;uint16_t steps=0;unsigned i;
 *status=cm_up_run(s,&io,&hw,n,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(io.superbank);mix(hw.channel11);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
