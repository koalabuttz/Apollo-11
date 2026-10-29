/* SPDX-License-Identifier: MIT; shared source/native input-only construction. */
#include "cm_interbank_complete_cases.h"
#include "cm_interbank_complete_inline_data.h"
agc_word cm_interbank_complete_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){
 static const agc_word pc[14]={04662,04666,04675,04701,04704,04711,04715,04740,04744,04753,04757,04773,04707,04710};
 static const agc_register raw[8]={0,0177777,040000,077777,0100000,0137777,1,0177776};
 uint16_t local=(uint16_t)n,i,q,physical;unsigned kind=cm_interbank_complete_kind(n),extra=(unsigned)((n-CM_INTERBANK_COMPLETE_BASE_CASES)%256u);agc_word literal;
 if(n>=CM_INTERBANK_COMPLETE_BASE_CASES)local=(uint16_t)(raw[extra&7u]^((extra&8u)?040u:0u));
 literal=(agc_word)(((uint32_t)local*977u)&AGC_MASK);
 *io=(cm_native_io){01234,05670,0770,03210,0765,0432,0,0,0123,0654,0456,0321,0777};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)(((uint32_t)i*29u+(local%16u)*251u+0x1234u)&AGC_MASK);
 q=(agc_word)(0100u+local%960u);s->words[0]=local;s->words[1]=(agc_register)((uint32_t)local*313u);s->words[3]=(agc_word)((local&7u)*0400u);s->words[4]=(agc_word)(((local>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(local&7u));s->words[5]=pc[kind];s->words[7]=0;
 physical=q<01400u?q:(uint16_t)(s->words[3]+(q&0377u));while(physical<0100u){++q;physical=q<01400u?q:(uint16_t)(s->words[3]+(q&0377u));}s->words[2]=q;
 s->words[0133]=s->words[072]=0710;s->words[0134]=s->words[073]=(agc_word)(((uint32_t)local*997u)&AGC_MASK);
 if(kind==CM_IC_BANKCALL||kind==CM_IC_IBNKCALL||kind==CM_IC_POSTJUMP||kind==CM_IC_USPRCADR)s->words[physical]=literal;
 io->superbank=(agc_word)(((local>>8)&7u)*020u);io->interrupt_enabled=(agc_word)((local>>5)&1u);
 if(n>=CM_INTERBANK_COMPLETE_BASE_CASES){
  s->words[2]=(agc_word)(02000u+(extra>>4)*0300u);
  if(kind==CM_IC_QJUMP)s->words[2]=(agc_word)(0100u+extra*3u);
  if(kind==CM_IC_SUPERSW){s->words[4]=(agc_word)(s->words[4]&056000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));}
  s->words[0133]=s->words[072]=(agc_word)(0100u+extra*3u);
  s->words[s->words[3]+0377u]=(agc_word)((extra>>4)%4u==0?0u:(extra>>4)%4u==1?077777u:(extra>>4)%4u==2?1u:077776u);
  io->superbank=(agc_word)(((extra>>1)&7u)*020u);io->interrupt_enabled=(agc_word)((extra>>3)&1u);
  if(kind==CM_IC_BANKCALL||kind==CM_IC_IBNKCALL||kind==CM_IC_POSTJUMP||kind==CM_IC_USPRCADR)literal=cm_interbank_complete_fixed_inline[extra];
 }
 return literal;
}
