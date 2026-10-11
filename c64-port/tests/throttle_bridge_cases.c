/* SPDX-License-Identifier: MIT */
#include "throttle_bridge_cases.h"
#include "agc_word.h"
void throttle_bridge_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint32_t seed=UINT32_C(0xb1234510)+n;uint16_t i;static const uint16_t raw[8]={0,0177777,1,0177776,040000,0137777,077777,0100000};
 for(i=0;i<APOLLO_SHARED_WORDS;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);s->words[i]=(uint16_t)((seed>>8)&077777u);}
 s->words[0]=raw[n&7u];s->words[1]=raw[(n>>3)&7u];s->words[2]=(uint16_t)(n&0177777u);
 s->words[3]=03400;s->words[4]=(uint16_t)(n<4096u?064000:062000);s->words[6]=(uint16_t)(s->words[4]|7u);s->words[7]=0;
 s->words[5]=(uint16_t)(n<4096u?06061:02367);s->words[0164]=(uint16_t)(n<4096u?03507:03510);s->words[0165]=064007;s->words[067]=0;
 if(n<4096u){s->words[0163]=0;s->words[023]=0;s->words[0154]=(uint16_t)(raw[(n>>6)&7u]&AGC_MASK);s->words[0155]=(uint16_t)(raw[(n>>9)&7u]&AGC_MASK);}
 else{s->words[0133]=03514;s->words[0134]=064000;s->words[03607]=04631;}
 io->superbank=(uint16_t)((n&3u)*020u);io->interrupt_enabled=(uint16_t)((n>>2)&1u);
}
