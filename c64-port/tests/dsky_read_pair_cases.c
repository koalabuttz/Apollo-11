/* SPDX-License-Identifier: MIT */
#include "dsky_read_pair_cases.h"
void dsky_read_pair_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission)
{
 uint16_t t=(uint16_t)(n%8192u),i,address;uint32_t seed=0x274700u+(n%16u);
 *mission=n<8192u?DSKY_NV_LM:DSKY_NV_CM;
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(0100u+t%07500u);
 s->words[7]=0;s->words[3]=(agc_word)(((t/16u)&7u)*0400u);s->words[4]=060000;s->words[6]=(agc_word)(060000u|(s->words[3]>>8));s->words[5]=*mission==DSKY_NV_LM?02747u:02655u;
 io->superbank=(agc_word)(0100u+((t/2u)&3u)*020u);io->interrupt_enabled=(agc_word)((t/8u)&1u);s->words[0140]=(agc_word)(1u+(t&1u));s->words[0117]=(agc_word)(t%3u);
 address=(agc_word)(0100u+t%03677u);if((address&0377u)==0377u)--address;
 s->words[0145]=(agc_word)(address<01400u?address:01400u+(address&0377u));s->words[0150u+s->words[0117]]=address;
 if(s->words[0140]==1u&&s->words[0145]>=01400u&&s->words[3]==0&&((s->words[0145]&0377u)<0100u))s->words[3]=0400;
 s->words[6]=(agc_word)(060000u|(s->words[3]>>8));
 {static const agc_word edge[8]={0,077777,1,077776,037777,040000,037776,040001};agc_word physical=s->words[0140]==2u?address:s->words[0145]<01400u?s->words[0145]:(agc_word)(s->words[3]+(s->words[0145]&0377u));if(t<256u&&physical>=0200u){s->words[physical]=edge[t%8u];s->words[physical+1u]=edge[(t+3u)%8u];}}
}
