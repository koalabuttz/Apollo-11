/* SPDX-License-Identifier: MIT */
#include "dsky_scale_cases.h"
#include "p66_bias_cases.h"
enum dsky_scale_entry dsky_scale_entry_for(uint16_t n){return (enum dsky_scale_entry)(n%3u);}
void dsky_scale_prepare(apollo_overlay_state *s,uint16_t n){
 static const uint16_t entries[]={02710,02715,02717};static const uint16_t corners[]={0,077777,037777,040000};uint16_t address,eb=(uint16_t)((n/3u)%8u);uint8_t i;
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)(eb*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000|eb);s->words[5]=entries[n%3u];s->words[0140]=(uint16_t)(1u+(n/18u)%2u);s->words[0117]=(uint16_t)((n/36u)%3u);s->words[0145]=(uint16_t)((eb<2 ? 01000u : 01400u)+n%0377u);
 for(i=0;i<3;++i)s->words[0150+i]=(uint16_t)(02262+((n+i)%3u)*0200u);
 address=s->words[0140]==2 ? s->words[0150+s->words[0117]] : (s->words[0145]<01400 ? s->words[0145] : (uint16_t)(eb*0400u+(s->words[0145]&0377u)));
 if(n<256){s->words[address]=corners[(n/3u)%4u];s->words[address+1u]=corners[(n/12u)%4u];s->words[0123]=corners[(n/48u)%4u];s->words[0124]=corners[(n/192u)%4u];}
 else if(n%5u==0){s->words[0123]=01031;s->words[0124]=21032;}
 else if(n%5u==1){s->words[0123]=01046;s->words[0124]=15700;}
}
