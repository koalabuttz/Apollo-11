/* SPDX-License-Identifier: MIT */
#include "dsky_fixed_display_cases.h"
void dsky_fixed_display_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t n,enum dsky_nv_mission *mission)
{uint16_t i,bank=(uint16_t)((n%40960u)/1024u),relative=(uint16_t)(n%1024u);uint32_t seed=0x3353u+n%16u;*mission=n<40960u?DSKY_NV_LM:DSKY_NV_CM;
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(0100u+n%07600u);s->words[3]=(agc_word)(((n>>4)&7u)*0400u);s->words[4]=062000;s->words[5]=03353;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[7]=0;
 s->words[01017]=(agc_word)((bank<32u?bank:bank-8u)*1024u+relative);s->words[01047]=(agc_word)((seed&077677u)|(bank>=32u?0100u:0u));
 io->superbank=(agc_word)(0100u+(n&3u)*020u);io->interrupt_enabled=(agc_word)((n>>2)&1u);
 /* Periodic clean+0 relay is a real formatter alarm frontier, not blank. */
 if(n%257u==0)s->words[01030]=0;
}
