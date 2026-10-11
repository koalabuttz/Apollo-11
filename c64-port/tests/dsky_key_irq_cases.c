/* SPDX-License-Identifier: MIT
 * Source-independent input images; no admission/kernel initialization. */
#include "dsky_key_irq_cases.h"
void dsky_key_irq_prepare(apollo_overlay_state *s,uint16_t n){uint16_t i,freecore=(uint16_t)((n/32u)%9u),eb=(uint16_t)((n/288u)%8u);uint32_t seed=n+517u;
 for(i=0;i<APOLLO_SHARED_WORDS;++i){seed=seed*1664525u+1013904223u;s->words[i]=(uint16_t)((seed>>16)&077777u);}
 s->words[0101]=(uint16_t[]){0,077777,040000,037777,1}[n%5u];if(n<128){static const uint16_t regs[]={0,077777,037777,040000,0100000,0177777,0140000,077776};s->words[0]=regs[(n/4u)%8u];s->words[1]=regs[(n/8u)%8u];s->words[2]=regs[n%8u];}if(n>=128){s->words[0]=(uint16_t)(s->words[0]|((n%2u)!=0 ? 0100000u : 0u));s->words[2]=(uint16_t)(s->words[2]|((n%3u)!=0 ? 0100000u : 0u));}s->words[3]=(uint16_t)(eb*0400u);s->words[4]=(uint16_t)(((n/128u)%32u)*02000u);s->words[6]=(uint16_t)(s->words[4]|eb);s->words[5]=04024;s->words[015]=(uint16_t)(04001u+n%03777u);s->words[7]=0;s->words[067]=freecore==0 ? 077777 : 0;s->words[0120]=0110;s->words[0121]=0;
 for(i=0;i<8;++i)s->words[0154+i*12u+11u]=i==freecore ? 077777 : (uint16_t)(035110u+i);
}

uint16_t dsky_key_irq_channel15(uint16_t n){return (uint16_t)(((uint32_t)n*151u+01000u)&077777u);}
