/* SPDX-License-Identifier: MIT
 * Source-independent input images; no admission/kernel initialization. */
#include "dsky_keycom_cases.h"
void dsky_keycom_prepare(apollo_overlay_state *s,uint16_t n){uint16_t i,freecore=(uint16_t)((n/32u)%9u),eb=(uint16_t)((n/288u)%8u);uint32_t seed=n+517u;
 for(i=0;i<APOLLO_SHARED_WORDS;++i){seed=seed*1664525u+1013904223u;s->words[i]=(uint16_t)((seed>>16)&077777u);}
 s->words[0101]=(uint16_t[]){0,077777,040000,037777,1}[n%5u];s->words[0]=(uint16_t)(n%32u);s->words[3]=(uint16_t)(eb*0400u);s->words[4]=010000;s->words[6]=(uint16_t)(010000u|eb);s->words[5]=03303;s->words[7]=0;s->words[067]=freecore==0 ? 077777 : 0;s->words[0120]=0110;s->words[0121]=0;
 for(i=0;i<8;++i)s->words[0154+i*12u+11u]=i==freecore ? 077777 : (uint16_t)(035110u+i);
}
