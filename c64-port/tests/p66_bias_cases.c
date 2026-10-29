/* SPDX-License-Identifier: MIT */
#include "p66_bias_cases.h"
void p66_bias_prepare(apollo_overlay_state *s,uint16_t n) {
 uint32_t seed=n+394;uint16_t i;static const uint16_t corners[]={0,077777,037777,040000};
 uint16_t base=(uint16_t)(0401u+(n%5u)*054u);
 for(i=0;i<APOLLO_SHARED_WORDS;++i){seed=seed*1664525u+1013904223u;s->words[i]=(uint16_t)((seed>>16)&077777u);}
 s->words[0]=0;s->words[1]=0;s->words[2]=01030;s->words[3]=03400;s->words[4]=062000;s->words[5]=02505;s->words[6]=062007;s->words[7]=0;
 s->words[067]=0;s->words[0120]=base;s->words[0166]=base;s->words[0167]=(uint16_t)(01000+base);s->words[base-1u]=0;
 if(n<64){for(i=0;i<6;++i)s->words[01452+i]=corners[(((uint32_t)n>>(2u*(i%3u)))+i)%4u];}
}
