/* SPDX-License-Identifier: MIT; shared source/native input-only construction. */
#include "lm_interbank_cases.h"
#include "lm_interbank_inline_data.h"
agc_word lm_interbank_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){
 static const agc_word pc[14]={04616,04622,04631,04635,04640,04645,04651,04674,04700,04707,04713,04727,04643,04644};
 static const agc_register raw[8]={0,0177777,040000,077777,0100000,0137777,1,0177776};
 uint16_t local=(uint16_t)n,i,q,physical;unsigned kind=lm_interbank_kind(n),extra=(unsigned)((n-LM_INTERBANK_BASE_CASES)%256u);agc_word literal;
 if(n>=LM_INTERBANK_BASE_CASES)local=(uint16_t)(raw[extra&7u]^((extra&8u)?040u:0u));
 literal=(agc_word)(((uint32_t)local*977u)&AGC_MASK);
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)(((uint32_t)i*29u+(local%16u)*251u+0x1234u)&AGC_MASK);
 q=(agc_word)(0100u+local%960u);s->words[0]=local;s->words[1]=(agc_register)((uint32_t)local*313u);s->words[3]=(agc_word)((local&7u)*0400u);s->words[4]=(agc_word)(((local>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(local&7u));s->words[5]=pc[kind];s->words[7]=0;
 physical=q<01400u?q:(uint16_t)(s->words[3]+(q&0377u));while(physical<0100u){++q;physical=q<01400u?q:(uint16_t)(s->words[3]+(q&0377u));}s->words[2]=q;
 s->words[0133]=s->words[072]=0710;s->words[0134]=s->words[073]=(agc_word)(((uint32_t)local*997u)&AGC_MASK);
 if(kind==LM_IB_BANKCALL||kind==LM_IB_IBNKCALL||kind==LM_IB_POSTJUMP||kind==LM_IB_USPRCADR)s->words[physical]=literal;
 io->superbank=(agc_word)(((local>>8)&7u)*020u);io->interrupt_enabled=(agc_word)((local>>5)&1u);
 if(n>=LM_INTERBANK_BASE_CASES){
  s->words[2]=(agc_word)(02000u+(extra>>4)*0300u);
  if(kind==LM_IB_QJUMP)s->words[2]=(agc_word)(0100u+extra*3u);
  if(kind==LM_IB_SUPERSW){s->words[4]=(agc_word)(s->words[4]&056000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));}
  s->words[0133]=s->words[072]=(agc_word)(0100u+extra*3u);
  s->words[s->words[3]+0377u]=(agc_word)((extra>>4)%4u==0?0u:(extra>>4)%4u==1?077777u:(extra>>4)%4u==2?1u:077776u);
  io->superbank=(agc_word)(((extra>>1)&7u)*020u);io->interrupt_enabled=(agc_word)((extra>>3)&1u);
  if(kind==LM_IB_BANKCALL||kind==LM_IB_IBNKCALL||kind==LM_IB_POSTJUMP||kind==LM_IB_USPRCADR)literal=lm_interbank_fixed_inline[extra];
 }
 return literal;
}
