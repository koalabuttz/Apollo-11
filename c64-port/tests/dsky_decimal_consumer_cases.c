/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_consumer_cases.h"
void dsky_decimal_consumer_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,agc_word *channel)
{
 static const agc_word routines[10]={1,2,3,4,5,6,7,10,11,13};static const agc_word corners[8]={0,077777,1,077776,037777,040000,037776,040001};uint32_t seed=0x2537u+n%16u;uint16_t i,t=(uint16_t)(n%2048u),count=(uint16_t)(t%3u);agc_word routine=routines[(t/3u)%10u],factor=(agc_word)((t/30u)%20u);
 *mission=n<2048u?DSKY_NV_LM:DSKY_NV_CM;
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02000u+t%02000u);s->words[3]=(agc_word)(((t>>3)&7u)*0400u);s->words[4]=062000;s->words[5]=02523;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[7]=0;io->superbank=(agc_word)(0100u+(t&3u)*020u);io->interrupt_enabled=(agc_word)((t>>2)&1u);*channel=(agc_word)((t*197u)&AGC_MASK);
 s->words[0140]=(agc_word)(1u+((t>>4)&1u));s->words[0145]=01000;s->words[0146]=(agc_word)(count<<10);s->words[0147]=(agc_word)((count<<10)|(routine<<5)|factor);s->words[0153]=0;
 if(s->words[0140]==2u){s->words[0147]=(agc_word)((factor<<10)|(factor<<5)|factor);s->words[0153]=(agc_word)((routine<<10)|(routine<<5)|routine);}
 for(i=0;i<3u;++i){s->words[0150u+i]=(agc_word)(01100u+2u*i);s->words[01000u+i]=corners[(t+i)&7u];s->words[01100u+2u*i]=corners[(t+i)&7u];s->words[01101u+2u*i]=(agc_word)((t*149u+i*23u)&AGC_MASK);}
 for(i=0;i<11u;++i)s->words[01023u+i]=(agc_word)((t&1u)?073777:037777);
 if((t&31u)==31u)s->words[01023u+(t>>5)%11u]=0;
}
