/* SPDX-License-Identifier: MIT */
#include "dsky_pinball_cases.h"
#include "p66_bias_cases.h"
enum dsky_pinball_entry dsky_pinball_entry_for(uint16_t n){return (enum dsky_pinball_entry)(n%3u);}
void dsky_pinball_prepare(apollo_overlay_state *s,uint16_t n){
 enum dsky_pinball_entry e=dsky_pinball_entry_for(n);uint16_t i;
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)(((n/3u)%8u)*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/3u)%8u));s->words[2]=(uint16_t)(04000+n%03777u);s->words[5]=e==DSKY_DSPIN ? 03322 : e==DSKY_11DSPIN ? 03404 : 02601;
 s->words[0]=(uint16_t)(n%11u);s->words[0143]=(uint16_t)((n/3u)%22u+((n/66u)%2u ? 020000u : 0u));s->words[0124]=e==DSKY_DSPIN ? (uint16_t)((n/3u)%32u) : (uint16_t)((n/6u)%2u*02000u);s->words[0777]=(uint16_t)((n/3u)%22u);
 for(i=0;i<11;++i){s->words[01023+i]=(uint16_t)(s->words[01023+i]|04000u);if((n/6u)%4u==0)s->words[01023+i]=04000;if((n/6u)%4u==1)s->words[01023+i]=077777;}
}
