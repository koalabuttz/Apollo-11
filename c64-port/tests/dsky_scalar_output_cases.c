/* SPDX-License-Identifier: MIT */
#include "dsky_scalar_output_cases.h"
void dsky_scalar_output_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,enum dsky_scalar_output_entry *entry,agc_word *channel)
{
    static const agc_word pc[2][6]={{02615,02677,02704,02623,02635,0},{02516,02603,02610,02524,0,02653}};
    static const agc_word corners[12]={0,077777,1,077776,037777,040000,037776,040001,AGC_MASK-016040,AGC_MASK-016040+1u,016040,AGC_MASK-016040-1u};
    uint32_t seed=0x261500u+(n%16u);uint16_t i,t=(uint16_t)(n%2048u),g=(uint16_t)(n/2048u),address;
    *mission=g<5u?DSKY_NV_LM:DSKY_NV_CM;g=(uint16_t)(g%5u);*entry=(enum dsky_scalar_output_entry)(*mission==DSKY_NV_CM&&g==4u?5u:g);
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*23u)&0177777u);
    s->words[7]=0;s->words[3]=(agc_word)(((t>>2)&7u)*0400u);s->words[4]=060000;s->words[6]=(agc_word)(060000u|(s->words[3]>>8));s->words[5]=pc[(unsigned)*mission][(unsigned)*entry];io->superbank=(agc_word)(0100u+((t&3u)*020u));io->interrupt_enabled=(agc_word)((t>>3)&1u);
    s->words[0154]=(agc_word)((t*97u)&AGC_MASK);if(t<12u)s->words[0154]=corners[t];s->words[0155]=(agc_word)((t*131u)&AGC_MASK);s->words[0156]=(agc_word)((t*193u)&AGC_MASK);
    if((t&31u)<12u){s->words[0123]=corners[t&31u];s->words[0124]=corners[(t+5u)%12u];}
    *channel=(agc_word)((t*53u)&AGC_MASK);
    if(*entry==DSKY_SCALAR_FRACTION){s->words[0140]=(agc_word)(1u+((t>>2)&1u));s->words[0117]=(agc_word)(t%3u);address=(uint16_t)(01000u+(t%6u)*0400u+(t%255u));s->words[0150u+s->words[0117]]=address;s->words[0145]=address<01400u?address:(agc_word)(01400u+(address&0377u));if(s->words[0140]==1u&&address>=01400u){s->words[3]=(agc_word)(address&03400u);s->words[6]=(agc_word)(060000u|(s->words[3]>>8));}}
}
