/* SPDX-License-Identifier: MIT */
#include "dsky_relay_cases.h"
#include "p66_bias_cases.h"
void dsky_relay_prepare(apollo_overlay_state *s,uint16_t n){
 uint16_t i,cnt=(uint16_t)(n%12u),pattern=(uint16_t)((n/12u)%14u);p66_bias_prepare(s,n);
 s->words[0]=(uint16_t)(n%16u);s->words[2]=02070;s->words[3]=01000;s->words[4]=014000;s->words[5]=02024;s->words[6]=014002;s->words[0776]=cnt<11 ? cnt : 077777;
 for(i=0;i<11;++i){uint16_t mag=(uint16_t)((s->words[01023+i]&01777u)|04000u);uint8_t dirty=(uint8_t)(pattern<11 ? i==pattern : pattern==11 ? 1 : pattern==12 ? (i&1u) : 0);s->words[01023+i]=dirty!=0 ? (uint16_t)(mag^077777u) : mag;}
}
