/* SPDX-License-Identifier: MIT */
#include "dsky_nv_continuation_cases.h"
void dsky_nv_continuation_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,enum dsky_nv_continuation_entry *entry)
{
    static const agc_word pc[2][9]={{04164,04170,04204,03570,03566,03575,03602,03604,03612},{04200,04204,04220,03570,03566,03575,03602,03604,03612}};
    uint32_t seed=0x550103u+(n%16u);uint16_t i,k=(uint16_t)(n%1024u);
    *mission=n<9216u?DSKY_NV_LM:DSKY_NV_CM;*entry=(enum dsky_nv_continuation_entry)((n/1024u)%9u);
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02000u+(k*11u)%05777u);
    s->words[7]=0;s->words[3]=(agc_word)(((k>>3)&7u)*0400u);s->words[4]=(agc_word)(((k>>6)&31u)*02000u);if(*entry>=DSKY_NV_ENTSET)s->words[4]=062000;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=pc[*mission][*entry];
    io->superbank=(agc_word)((k&7u)*020u);if(*entry>=DSKY_NV_ENTSET)io->superbank|=0100u;io->interrupt_enabled=(agc_word)((k>>3)&1u);
    s->words[01037]=(agc_word)(02000u+(k*13u)%06000u);s->words[01040]=(agc_word)(((k&31u)*02000u)|(((k>>5)&7u)*020u)|((k>>8)&7u));if(*entry==DSKY_NV_COMMON)s->words[0]=(agc_register)(k&1u);
}
