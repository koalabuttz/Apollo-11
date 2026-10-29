/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_general_cases.h"
void dsky_decimal_general_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,uint8_t *rounded)
{
    static const agc_word corner[8]={0,077777,037777,040000,1,077776,037776,040001};
    uint32_t seed=0x322600u+(n%16u);uint16_t i,t=(uint16_t)(n%8192u),slot;
    *mission=n<8192u?DSKY_NV_LM:DSKY_NV_CM;*rounded=(uint8_t)((t>>2)&1u);
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02000u+(t%02000u));
    s->words[7]=0;s->words[3]=(agc_word)(((t>>3)&7u)*0400u);s->words[4]=060000;s->words[6]=(agc_word)(060000u|(s->words[3]>>8));s->words[5]=*mission==DSKY_NV_LM?(*rounded?03226u:03262u):(*rounded?03133u:03167u);io->superbank=(agc_word)(0100u+((t&3u)*020u));io->interrupt_enabled=(agc_word)((t>>3)&1u);
    s->words[0777]=(agc_word)(14u-5u*(t%3u));s->words[0154]=(agc_word)((t*197u)&AGC_MASK);s->words[0155]=(agc_word)(((t*241u)&037777u)|(s->words[0154]&AGC_SIGN));s->words[0156]=(agc_word)((t*149u)&AGC_MASK);
    if((t&31u)<8u){s->words[0154]=corner[t&7u];s->words[0155]=corner[t&7u];}
    for(i=0;i<11u;++i)s->words[01023u+i]=(agc_word)((t&1u)?073777u:037777u);
    slot=(uint16_t)((t>>5)%12u);if(slot<11u)s->words[01023u+slot]=0;
    if((t&63u)==63u){s->words[01016]=037777;for(i=0;i<11u;++i)s->words[01023u+i]=037777;}
}
