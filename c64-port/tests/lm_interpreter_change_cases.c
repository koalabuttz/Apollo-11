/* SPDX-License-Identifier: MIT */
#include "lm_interpreter_change_cases.h"
void lm_interpreter_change_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 static const uint16_t raw16[8]={0,0177777,1,0177776,040000,0137777,077777,0100000};
 static const uint16_t push[8]={0,077777,1,077776,0401,07376,037777,040000};
 static const uint16_t ovf[4]={0,077777,1,077776};
 static const uint16_t vac[8]={0110,0401,0455,0531,0605,0661,0,1};
 uint16_t i,j,slot=(uint16_t)(1u+n%7u),off=(uint16_t)(slot*12u);uint32_t seed=(uint32_t)n+98317u;
 for(i=0;i<APOLLO_SHARED_WORDS;++i)s->words[i]=(uint16_t)((i*271u+(n&15u)*983u)&AGC_MASK);
 for(i=0;i<8u;++i)for(j=0;j<12u;++j){seed=seed*1664525u+1013904223u;s->words[0154u+i*12u+j]=(uint16_t)((seed>>16)&AGC_MASK);}
 s->words[0]=raw16[n%8u];s->words[1]=raw16[(n/8u)%8u];s->words[2]=raw16[(n/64u)%8u];
 s->words[3]=(uint16_t)((n/512u)%8u*0400u);s->words[4]=064000;s->words[5]=05126;s->words[6]=(uint16_t)(064000u|(s->words[3]>>8));s->words[7]=0;
 s->words[023]=0;s->words[067]=off;s->words[0164]=(uint16_t)(03353u+n%0110u);s->words[0165]=s->words[6];s->words[0166]=push[(n/2u)%8u];s->words[0167]=(uint16_t)(022000u|vac[(n/16u)%8u]);s->words[0120]=(uint16_t)(s->words[0167]&0777u);s->words[0121]=ovf[(n/128u)%4u];
 s->words[0164u+off]=(n&1u)?agc_negate((uint16_t)(03360u+slot)):(uint16_t)(02000u+slot);
 s->words[0165u+off]=(uint16_t)((n&1u?064000u:062000u)|((n/1024u)%8u)|((n/2048u)%8u*020u));
 s->words[0166u+off]=push[(n/32u)%8u];s->words[0167u+off]=(uint16_t)(026000u|vac[(n/256u)%8u]);
 io->superbank=(uint16_t)((n/2048u)%8u*020u);io->interrupt_enabled=(uint16_t)((n/4096u)%2u);
}

uint16_t lm_interpreter_change_channel11(uint16_t n){return (uint16_t)(((uint32_t)n*151u+04100u)&AGC_MASK);}
uint16_t lm_interpreter_change_channel14(uint16_t n){return (uint16_t)(((uint32_t)n*73u+01234u)&0777u);}
