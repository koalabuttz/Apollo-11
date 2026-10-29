/* SPDX-License-Identifier: MIT */
#include "dsky_nv_blank_cases.h"
void dsky_nv_blank_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,uint16_t n,enum dsky_nv_mission *mission)
{
    static const agc_word values[8]={0,077777,1,077776,037777,040000,01234,076543};
    uint32_t seed=0x351600u+(n%16u);uint16_t i,k=(uint16_t)(n%1024u),option=(uint16_t)((n/1024u)%4u);
    *mission=n<4096u?DSKY_NV_LM:DSKY_NV_CM;
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=option;s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*11u)&0177777u);
    s->words[7]=0;s->words[3]=(agc_word)(((k>>3)&7u)*0400u);s->words[4]=062000;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[5]=03516;
    io->superbank=(agc_word)(0100u+((k&3u)*020u));io->interrupt_enabled=(agc_word)((k>>2)&1u);*channel11=(agc_word)((seed>>7)&AGC_MASK);
    s->words[0123]=(agc_word)(077776u-option);s->words[01016]=values[(k>>3)&7u];
    for(i=0;i<11u;++i){if(k<512u)s->words[01023u+i]=values[((k>>6)+i+(k&7u))&7u];}
    /* Explicit clean positive/zero sweeps, overflow-corrected NOUT cases,
     * and -zero at each loop position rather than only the first relay. */
    if(k<128u){for(i=0;i<11u;++i)s->words[01023u+i]=1;s->words[01016]=values[k&7u];if(k>=8u)s->words[01023u+((k>>3)%11u)]=values[k&7u];}
}
