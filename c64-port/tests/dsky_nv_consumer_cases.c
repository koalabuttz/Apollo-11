/* SPDX-License-Identifier: MIT */
#include "dsky_nv_consumer_cases.h"
void dsky_nv_consumer_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,uint16_t n,enum dsky_nv_mission *mission)
{
    uint32_t seed=0x415400u+(n%16u);uint16_t i,k=(uint16_t)(n%128u),group=(uint16_t)((n/128u)%7u);agc_word verb=(agc_word)(1u+k%99u),noun=(agc_word)(1u+(k*31u)%99u);
    *mission=n<896u?DSKY_NV_LM:DSKY_NV_CM;
    for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
    if(group==0)noun=0;else if(group==1)verb=0;
    s->words[0]=group>=3u?agc_extend((agc_word)(077776u-(group-3u))):agc_extend((agc_word)((verb<<7)|noun));
    s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02400u+k);
    s->words[7]=0;s->words[3]=(agc_word)((k&7u)*0400u);s->words[4]=(agc_word)(((k>>2)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=(agc_word)((*mission==DSKY_NV_LM?04154u:04170u)+((k>>3)&1u));
    io->superbank=(agc_word)(((k>>2)&7u)*020u);io->interrupt_enabled=(agc_word)((k>>6)&1u);*channel11=(agc_word)((seed>>7)&AGC_MASK);
    s->words[01012]=0;s->words[01021]&=057777u;
    for(i=0;i<11u;++i)s->words[01023u+i]=073777;
    if(group>=3u){static const agc_word old[8]={0,077777,1,077776,037777,040000,01234,073777};s->words[01023u+(k%11u)]=old[(k>>3)&7u];s->words[01016]=old[k&7u];}
}
uint8_t dsky_nv_consumer_count(uint16_t n){return (uint8_t)(((n/128u)%7u)==2u?5u:3u);}
enum dsky_nv_consumer_stage dsky_nv_consumer_stage_at(uint16_t n,uint8_t phase){if(phase==0)return (n&8u)?DSKY_NV_CONSUMER_MONITOR:DSKY_NV_CONSUMER_CALL;return (phase&1u)?DSKY_NV_CONSUMER_SERVICE:DSKY_NV_CONSUMER_CONTINUE;}
