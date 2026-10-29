/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_consumer_cases.h"
void dsky_monitor_consumer_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n,enum dsky_nv_mission *mission,agc_word *channel)
{static const agc_word corners[8]={0,077777,1,077776,037777,040000,037776,040001};uint16_t i,t=(uint16_t)(n%128u),noun=(t&1u)?60u:((t&14u)==14u?42u:9u),verb=(t&1u)?16u:(agc_word)(11u+(t/2u)%5u);uint32_t seed=0x3321u+n%16u;*mission=n<128u?DSKY_NV_LM:DSKY_NV_CM;
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02000u+t);s->words[3]=(agc_word)(((t>>4)&7u)*0400u);s->words[4]=062000;s->words[5]=03321;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[7]=0;
 io->control.superbank=(agc_word)(0100u+(t&3u)*020u);io->control.interrupt_enabled=(agc_word)((t>>2)&1u);io->channel11=(agc_word)((t*97u)&AGC_MASK);io->wrote_channel11=(agc_word)((t>>3)&1u);*channel=(agc_word)((t*173u)&AGC_MASK);
 s->words[01020]=(agc_word)(verb*128u+noun);s->words[01021]=020000;s->words[01012]=0;s->words[01022]=(agc_word)(((t&2u)?(17u+t%20u)*128u:0u)|(t&7u));s->words[01001]=verb;s->words[01002]=noun;s->words[0115]=0;s->words[067]=0;
 for(i=0;i<11u;++i)s->words[01023u+i]=073777;
 s->words[02262]=corners[t&7u];s->words[02263]=corners[(t+1u)&7u];s->words[03473]=corners[(t+2u)&7u];s->words[03474]=corners[(t+3u)&7u];s->words[03534]=corners[(t+4u)&7u];s->words[03535]=corners[(t+5u)&7u];
 if(t%31u==29u)s->words[01012]=1;
 if(t%31u==30u)s->words[01021]=077777;
}
