/* SPDX-License-Identifier: MIT */
#include "lm_wake_cases.h"
#include "throttle_bridge_cases.h"
void lm_wake_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint8_t i,slot=(uint8_t)(n%8u);agc_word wake=(agc_word)(((uint32_t)n*13u)&077777u);
 throttle_bridge_input(s,io,n);s->words[0]=agc_extend(wake);s->words[2]=02570;s->words[3]=(uint16_t)((n%8u)*0400u);s->words[4]=(uint16_t)(((n/8u)%32u)*02000u);s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));s->words[5]=05137;s->words[0167]=(uint16_t)(020000u+(n&0777u));s->words[067]=(agc_word)(((n/64u)%8u)*12u);
 if((n/512u)%2u)s->words[067]=AGC_NEG_ZERO;
 io->superbank=(uint16_t)((n%8u)*020u);io->interrupt_enabled=(uint16_t)(n%2u);
 for(i=1;i<8u;++i){uint16_t a=(uint16_t)(0154u+i*12u);s->words[a+8u]=(agc_word)((wake+17u+i)&077777u);s->words[a+9u]=(agc_word)((n+i)%8u|(((n+i)%8u)*020u));s->words[a+11u]=(agc_word)(01000u*(((uint32_t)n*13u+i*7u)%31u+1u)+i*17u);if((n+i)%3u==0u)s->words[a+11u]=AGC_NEG_ZERO;else if((n+i)%3u==1u)s->words[a+11u]=agc_negate(s->words[a+11u]);}
 if(slot){uint16_t a=(uint16_t)(0154u+slot*12u);s->words[a+8u]=wake;s->words[a+11u]=agc_negate((agc_word)(01000u*((n/8u)%31u+1u)+0110u));}
 if(n<128u){s->words[0]=agc_extend((agc_word)(n&1u?AGC_NEG_ZERO:0));for(i=1;i<8u;++i)s->words[0167u+i*12u]=AGC_NEG_ZERO;if(slot){s->words[0164u+slot*12u]=(agc_word)(n&2u?AGC_NEG_ZERO:0);s->words[0167u+slot*12u]=agc_negate(033110);if(n&4u&&slot<7u){s->words[0164u+(slot+1u)*12u]=agc_correct(s->words[0]);s->words[0167u+(slot+1u)*12u]=agc_negate(037110);}}}
}
