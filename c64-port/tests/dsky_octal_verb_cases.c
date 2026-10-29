/* SPDX-License-Identifier: MIT */
#include "dsky_octal_verb_cases.h"
void dsky_octal_verb_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,uint8_t *entry)
{
 uint16_t t=(uint16_t)(n%8192u),i,verb=(uint16_t)(1u+(t%5u));uint32_t seed=0x236500u+(n%16u);
 static const agc_word pc[5]={02365,02373,02400,02360,02353};
 *mission=n<8192u?DSKY_NV_LM:DSKY_NV_CM;*entry=(uint8_t)((t/256u)%3u);
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*23u)&0177777u);
 s->words[035]=s->words[036]=s->words[047]=s->words[050]=s->words[051]=s->words[052]=0;s->words[7]=0;s->words[3]=(agc_word)(((t/16u)&7u)*0400u);s->words[4]=062000;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[5]=*entry==2u?02504u:*entry?02422u:pc[verb-1u];
 io->superbank=(agc_word)(0100u+((t/8u)&3u)*020u);io->interrupt_enabled=(agc_word)((t/4u)&1u);
 s->words[01001]=verb;s->words[0140]=(agc_word)(1u+(t&1u));s->words[0145]=(agc_word)(0400u+(t%01000u));s->words[0146]=(agc_word)(((t/2u)&017u)<<10);s->words[0147]=(agc_word)((((t/2u)&017u)<<10)|(((t/32u)%14u)<<5));s->words[0122]=(agc_word)(t%3u);s->words[01017]=(agc_word)(t%01000u);
 if(t<128u)s->words[0145]=(agc_word)(0100u+t);
 if(!*entry&&verb==1u&&t%31u==0u){s->words[0140]=1;s->words[0146]=077776;s->words[0147]&=057777u;}
}
