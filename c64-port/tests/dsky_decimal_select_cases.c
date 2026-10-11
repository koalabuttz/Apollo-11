/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_select_cases.h"
void dsky_decimal_select_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission)
{
    uint32_t seed=0x253700u+(n%16u);uint16_t i,t=(uint16_t)(n%4096u),u=t;
    agc_word k=(agc_word)(u%3u),mix,factor,routine;unsigned shift;
    *mission=n<4096u?DSKY_NV_LM:DSKY_NV_CM;u=(uint16_t)(u/3u);mix=(agc_word)(1u+u%2u);u=(uint16_t)(u/2u);routine=(agc_word)(u%14u);u=(uint16_t)(u/14u);factor=(agc_word)(u%(*mission==DSKY_NV_LM?29u:20u));
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_register)((seed*23u)&0177777u);
    s->words[7]=0;s->words[3]=(agc_word)(((t>>2)&7u)*0400u);s->words[4]=062000;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[5]=02537;io->superbank=(agc_word)(0100u+((t&3u)*020u));io->interrupt_enabled=(agc_word)((t>>3)&1u);
    if((t&31u)<8u){static const agc_word corners[8]={0,077777,037777,040000,1,077776,040001,037776};s->words[01003u+k]=corners[t&7u];}
    s->words[0140]=mix;s->words[0117]=k;shift=5u*k;
    if(mix==1u)s->words[0147]=(agc_word)((s->words[0147]&076000u)|((unsigned)routine<<5)|factor);
    else {s->words[0147]=(agc_word)((s->words[0147]&~(037u<<shift))|((unsigned)factor<<shift));s->words[0153]=(agc_word)((s->words[0153]&~(037u<<shift))|((unsigned)routine<<shift));}
}
