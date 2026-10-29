/* SPDX-License-Identifier: MIT */
#include "cm_display_rest_cases.h"
void cm_display_rest_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n)
{
    static const agc_word flags[8]={0,077777,1,077776,037777,040000,01234,076543};
    uint32_t seed=0x550101u+(n%16u);uint16_t i,k=(uint16_t)(n%1024u);agc_word *p=(agc_word *)io;
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    for(i=0;i<sizeof *io/sizeof *p;++i){seed=seed*1664525u+1013904223u;p[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*23u+n*127u)&0177777u);
    s->words[7]=0;s->words[3]=(agc_word)(((k>>3)&7u)*0400u);s->words[4]=020000;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));s->words[5]=n<1024u?03343u:n<2048u?03467u:03473u;s->words[0164]=(agc_word)(k%3u);s->words[0162]=(agc_word)(k*71u&AGC_MASK);s->words[01042]=flags[k&7u];io->superbank=(agc_word)((k&7u)*020u);io->interrupt_enabled=(agc_word)((k>>3)&1u);
}
