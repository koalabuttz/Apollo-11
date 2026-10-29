/* SPDX-License-Identifier: MIT */
#include "dsky_nv_return_cases.h"
void dsky_nv_return_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission)
{
    uint32_t seed=0x550099u+(n%16u);uint16_t i,k=(uint16_t)(n%4096u);
    *mission=n<4096u?DSKY_NV_LM:DSKY_NV_CM;
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*23u+n*127u)&0177777u);
    s->words[7]=0;s->words[3]=(agc_word)(((k>>3)&7u)*0400u);s->words[4]=(agc_word)(((k>>6)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=*mission==DSKY_NV_LM?04202u:04216u;
    io->superbank=(agc_word)((k&7u)*020u);io->interrupt_enabled=(agc_word)((k>>3)&1u);
    s->words[01037]=(agc_word)(02000u+(k*11u)%06000u);
    s->words[01040]=(agc_word)(((k&31u)*02000u)|(((k>>5)&7u)*020u)|((k>>8)&7u));
}
