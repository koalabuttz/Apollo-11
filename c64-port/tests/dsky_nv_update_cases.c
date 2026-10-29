/* SPDX-License-Identifier: MIT */
#include "dsky_nv_update_cases.h"
void dsky_nv_update_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,enum dsky_nv_update_entry *entry)
{
    static const agc_word pc[4]={02337,02340,02317,02320};uint32_t seed=0x550104u+(n%16u);uint16_t i,k=(uint16_t)(n%1024u);
    *mission=n<4096u?DSKY_NV_LM:DSKY_NV_CM;*entry=(enum dsky_nv_update_entry)((n/1024u)%4u);
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02000u+(k*11u)%06000u);
    s->words[7]=0;s->words[3]=(agc_word)(((k>>3)&7u)*0400u);s->words[4]=062000;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[5]=pc[*entry];io->superbank=0100;io->interrupt_enabled=(agc_word)((k>>3)&1u);
    if(*entry==DSKY_NV_VERB_STORE||*entry==DSKY_NV_NOUN_STORE)s->words[0]=(agc_word)(k%100u);else s->words[*entry==DSKY_NV_NOUN_UPDATE?01002u:01001u]=(agc_word)(k%100u);
    for(i=0;i<11u;++i)if(!s->words[01023u+i])s->words[01023u+i]=073777;
}
