/* SPDX-License-Identifier: MIT */
#include "dsky_test_noun_cases.h"
void dsky_test_noun_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission)
{static const agc_word edge[8]={0,077777,1,077776,037777,040000,037776,040001};uint16_t i,t=(uint16_t)(n%8192u),noun=(uint16_t)(t%101u);uint32_t seed=0x2046u+n%16u;*mission=n<8192u?DSKY_NV_LM:DSKY_NV_CM;
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)((t*271u)&AGC_MASK);s->words[3]=(agc_word)(((t>>4)&7u)*0400u);s->words[4]=062000;s->words[5]=02046;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[7]=0;s->words[01002]=noun==100u?AGC_NEG_ZERO:noun;s->words[01017]=edge[(t/101u)%8u];io->superbank=(agc_word)(0100u+(t&3u)*020u);io->interrupt_enabled=(agc_word)((t>>2)&1u);}
