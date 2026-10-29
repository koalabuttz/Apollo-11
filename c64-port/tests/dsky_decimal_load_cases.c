/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_load_cases.h"
void dsky_decimal_load_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission)
{
    uint32_t seed=0x252300u+(n%16u);uint16_t i,k=(uint16_t)(n%4096u);agc_word field=(agc_word)((((k>>3)&7u)<<2)|(k%3u));
    *mission=n<4096u?DSKY_NV_LM:DSKY_NV_CM;
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*23u)&0177777u);
    s->words[7]=0;s->words[3]=(agc_word)(((k>>3)&7u)*0400u);s->words[4]=062000;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[5]=02523;io->superbank=(agc_word)(0100u+((k&3u)*020u));io->interrupt_enabled=(agc_word)((k>>3)&1u);
    s->words[0140]=(agc_word)(1u+((k>>2)&1u));s->words[s->words[0140]==1u?0147u:0146u]=(agc_word)((s->words[s->words[0140]==1u?0147u:0146u]&01777u)|((unsigned)field<<10));
    s->words[0145]=(agc_word)(0100u+k%(01775u-0100u+1u));
    if(k>=64u&&k<128u){s->words[3]=0;s->words[6]=062000;s->words[0145]=(agc_word)(01400u+(k&31u));}
    if(k<64u){static const agc_word addresses[8]={0117,0122,0140,0145,0146,01003,01400,01775};s->words[0145]=addresses[k&7u];}
}
