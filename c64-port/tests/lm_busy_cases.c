/* SPDX-License-Identifier: MIT */
#include "lm_busy_cases.h"
#include "throttle_bridge_cases.h"
void lm_busy_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){throttle_bridge_input(s,io,n);s->words[0]=agc_extend((agc_word)(((uint32_t)n*13u)&AGC_MASK));s->words[5]=04442;s->words[3]=(uint16_t)((n%8u)*0400u);s->words[4]=(uint16_t)(((n/8u)%32u)*02000u);s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));io->superbank=(uint16_t)((n%8u)*020u);io->interrupt_enabled=(uint16_t)(n%2u);*channel=(agc_word)(((uint32_t)n*977u)&AGC_MASK);
 if(n%4u==0u)s->words[01042]=s->words[01043]=0;else if(n%4u==1u){s->words[01042]=AGC_NEG_ZERO;s->words[01043]=0;}else if(n%4u==2u)s->words[01042]=0;
 if(s->words[01043]==AGC_NEG_ZERO)s->words[01043]=1;
 if(n<32u){static const agc_word values[8]={0,AGC_NEG_ZERO,1,077776,037777,040000,01234,AGC_NEG_ZERO};s->words[0]=agc_extend(values[n%8u]);}
}
