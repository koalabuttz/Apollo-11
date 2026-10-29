/* SPDX-License-Identifier: MIT */
#include "dsky_nv_busy_cases.h"
void dsky_nv_busy_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,uint16_t n,enum dsky_nv_mission *mission,enum dsky_nv_busy_entry *entry)
{
    static const agc_word flags[8]={0,077777,1,077776,037777,040000,01234,076543};
    uint32_t seed=0x550100u+(n%16u);uint16_t i,k=(uint16_t)(n%1024u);agc_word pre;
    *mission=n<5120u?DSKY_NV_LM:DSKY_NV_CM;*entry=(enum dsky_nv_busy_entry)((n/1024u)%5u);pre=*mission==DSKY_NV_LM?04437u:04453u;
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=agc_extend((agc_word)((seed+k*113u)&AGC_MASK));s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02000u+(k*11u)%02000u);
    s->words[7]=0;s->words[3]=(agc_word)(((k>>3)&7u)*0400u);s->words[4]=(agc_word)(((k>>6)&31u)*02000u);if(*entry==DSKY_NV_BUSY_INNER)s->words[4]=010000;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));
    s->words[5]=*entry==DSKY_NV_ENDIDLE?(*mission==DSKY_NV_LM?04207u:04223u):*entry==DSKY_NV_BUSY?(agc_word)(pre+3u):*entry==DSKY_NV_PREBUSY?pre:*entry==DSKY_NV_WAIT?(agc_word)(pre+6u):(*mission==DSKY_NV_LM?02647u:02577u);
    io->superbank=(agc_word)((k&7u)*020u);io->interrupt_enabled=(agc_word)((k>>3)&1u);*channel11=(agc_word)((seed>>7)&AGC_MASK);
    s->words[01042]=flags[(k>>4)&7u];s->words[01043]=flags[k&7u];if(s->words[01043]==AGC_NEG_ZERO)s->words[01043]=0;
    s->words[01012]=(agc_word)((k>>6)&7u);s->words[01021]=(agc_word)((s->words[01021]&057777u)|((k&8u)?020000u:0u));
}
