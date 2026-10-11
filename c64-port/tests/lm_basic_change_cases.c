/* SPDX-License-Identifier: MIT */
#include "lm_basic_change_cases.h"
void lm_basic_change_input(apollo_overlay_state*m,p66_bias_io*io,uint16_t n){uint16_t i,offset=(uint16_t)(12u*(1u+(n/2u)%7u)),slot=(uint16_t)(n/14u),oldbank;uint32_t seed=UINT32_C(0x995122)+n/224u;
 for(i=0;i<2048u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);m->words[i]=(uint16_t)((seed>>11)&AGC_MASK);}m->words[7]=m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054u;++i)m->words[i]=0;
 io->superbank=(uint16_t)((slot%8u)*020u);io->interrupt_enabled=(uint16_t)((n/1792u)%2u);m->words[3]=(uint16_t)((slot%8u)*0400u);m->words[4]=(uint16_t)(((slot/8u)%32u)*02000u);m->words[6]=(uint16_t)(m->words[4]|(m->words[3]>>8));oldbank=m->words[6];m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);if(n%8u==0)m->words[1]=(uint16_t)((n/8u)*97u);m->words[2]=(uint16_t)(0700u+slot%0100u);m->words[067]=offset;m->words[0167]=(uint16_t)(012000u+0401u);m->words[0167u+offset]=(uint16_t)(037000u+0511u);m->words[0121]=(uint16_t)((slot/256u)%3u==0?0:(slot/256u)%3u==1?1:AGC_NEG_ZERO);m->words[0166]=(uint16_t)(0401u+slot%40u);m->words[0166u+offset]=(uint16_t)(0511u+slot%40u);if(slot&1u)m->words[0166u+offset]=agc_negate(m->words[0166u+offset]);
 m->words[0164u+offset]=(slot&2u)?agc_negate(03360):02077;m->words[0165u+offset]=(uint16_t)((((slot/4u)%32u)*02000u)|((slot%8u)*020u)|(slot%8u));m->words[5]=05122;
 if(n&1u){m->words[0]=agc_extend(oldbank);m->words[1]=agc_extend(m->words[2]);m->words[6]=02001;m->words[3]=0400;m->words[4]=02000;m->words[5]=02706;}
}
