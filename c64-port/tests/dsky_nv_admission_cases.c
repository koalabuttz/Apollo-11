/* SPDX-License-Identifier: MIT */
#include "dsky_nv_admission_cases.h"
static uint32_t random_word(uint32_t *seed){*seed=*seed*1664525u+1013904223u;return *seed;}
void dsky_nv_admission_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,enum dsky_nv_entry *entry)
{
    static const agc_word special[12]={0,077777,077776,077775,077774,077773,01450,01525,050,01400,037777,1};
    uint32_t seed=0x3600055u+(n%16u);uint16_t i,local=(uint16_t)(n%1024u);agc_word a;
    *mission=n<4096u?DSKY_NV_LM:DSKY_NV_CM;*entry=(enum dsky_nv_entry)((n/1024u)%4u);
    for(i=0;i<2048u;++i)s->words[i]=(agc_word)((random_word(&seed)>>11)&AGC_MASK);
    s->words[7]=0;s->words[3]=(agc_word)(((local>>3)&7u)*0400u);s->words[4]=(agc_word)(((local>>6)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));
    s->words[2]=(agc_word)(02000u+(local*11u)%06000u);s->words[1]=(agc_register)(random_word(&seed)&0177777u);
    io->superbank=(agc_word)((local&7u)*020u);io->interrupt_enabled=(agc_word)((local>>3)&1u);
    a=local<12u?special[local]:(agc_word)((random_word(&seed)>>8)&037777u);
    s->words[0]=agc_extend(a);s->words[0123]=a;
    s->words[01021]&=057777u;s->words[01012]=0;
    if(local>=16u&&local&8u){s->words[01012]=(agc_word)(local&7u);if(local&16u)s->words[01021]|=020000u;}
    if(*entry<=DSKY_NV_NVMONOPT)s->words[5]=(agc_word)((*mission==DSKY_NV_LM?04154u:04170u)+(*entry==DSKY_NV_NVMONOPT?1u:0u));
    else{s->words[4]=062000;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[5]=*entry==DSKY_NV_NVSUB1?03544u:03571u;io->superbank=(agc_word)(0100u|((local&3u)*020u));if(*entry==DSKY_NV_NVSUB2){s->words[0157]=(agc_word)(local&0177u);s->words[0160]=(agc_word)((local>>3)&0177u);}}
}
