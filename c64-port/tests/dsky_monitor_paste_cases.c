/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_paste_cases.h"
void dsky_monitor_paste_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,enum dsky_monitor_paste_stage *stage)
{uint16_t i,t=(uint16_t)(n%8192u),paste=n<8192u?04124u:04140u,after=n<8192u?04136u:04152u,blank=n<8192u?04142u:04156u;uint32_t seed=0x4124u+n%16u;*mission=n<8192u?DSKY_NV_LM:DSKY_NV_CM;*stage=(enum dsky_monitor_paste_stage)(t%3u);
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(0100u+t%07676u);s->words[3]=(agc_word)(((t>>4)&7u)*0400u);s->words[4]=062000;s->words[5]=*stage==DSKY_MONITOR_PASTE_BEGIN?(t&1u?paste:0136u):*stage==DSKY_MONITOR_PASTE_UPDATED?(n<8192u?04631u:04675u):(agc_word)(blank+(t&1u));s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[7]=0;s->words[0136]=paste;s->words[0133]=after;s->words[0134]=062000;
 s->words[01020]=(agc_word)(((t/3u)%100u)*128u+(seed&0177u));s->words[01022]=(agc_word)(((t&3u)?(t/12u)%100u*128u:0u)|(seed&040177u));io->superbank=(agc_word)(0100u+(t&3u)*020u);io->interrupt_enabled=(agc_word)((t>>2)&1u);}
