/* SPDX-License-Identifier: MIT */
#include "dsky_sign_output_cases.h"
void dsky_sign_output_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission,uint8_t *negative)
{
 uint16_t t=(uint16_t)(n%8192u),i,minus;uint32_t seed=0x241300u+(n%16u);
 *mission=n<8192u?DSKY_NV_LM:DSKY_NV_CM;*negative=(uint8_t)(t&1u);
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(0100u+t%07500u);
 s->words[7]=0;s->words[3]=(agc_word)(((t/16u)&7u)*0400u);s->words[4]=060000;s->words[6]=(agc_word)(060000u|(s->words[3]>>8));s->words[5]=(agc_word)((*mission==DSKY_NV_LM?02413u:02314u)+(*negative?020u:0u));s->words[0777]=(agc_word)(t%15u);
 io->superbank=(agc_word)(0100u+((t/2u)&3u)*020u);io->interrupt_enabled=(agc_word)((t/8u)&1u);minus=s->words[0777]<5u?0u:s->words[0777]<10u?3u:5u;
 if(t%7u==0u)s->words[01023u+minus]=0;
 if(t%11u==0u)s->words[01024u+minus]=0;
 if(t<128u)s->words[01016]=(agc_word)(t<64u?037777u:077777u);
}
