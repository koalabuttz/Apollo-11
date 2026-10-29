/* SPDX-License-Identifier: MIT */
#include "cm_restart_timing_cases.h"
#include "agc_word.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_rstime_input(apollo_overlay_state *s,uint16_t n){
 unsigned i,row=n%2048u;uint32_t seed=0x71357u+n;
 static const agc_word corners[]={0,1,2,037776,037777,040000,040001,077776,077777};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[7]=0;s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;
 s->words[035]=0;s->words[036]=0;s->words[047]=0;
 for(i=050;i<=054;++i)s->words[i]=0;
 s->words[0]=agc_extend(row<81u?corners[row%9u]:s->words[0]);
 s->words[1]=agc_extend(row<81u?corners[row/9u]:s->words[1]);
 if(n>=2048u&&row<81u){s->words[024]=0;s->words[025]=0;s->words[01135]=0;s->words[01136]=0;}
 s->words[0155]=(agc_word)((n%6u)*2u);
 s->words[5]=n<2048u?03573:03701;s->words[0704]=05245;
}
