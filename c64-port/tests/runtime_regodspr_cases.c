/* SPDX-License-Identifier: MIT */
#include "runtime_regodspr_cases.h"
#include "regodspr_cases.h"
void runtime_regodspr_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint8_t i,free_slot=(uint8_t)(n%8u),pending_slot=(uint8_t)((n/8u)%8u);
 regodspr_input(s,io,(uint16_t)(n&1023u));
 s->words[0167]=(agc_word)(((n/8u)%31u+1u)*01000u+0401u);
 for(i=1;i<8u;++i)s->words[0154u+12u*i+11u]=(agc_word)((((uint32_t)n*17u+i)%31u+1u)*01000u+0110u);
 if(free_slot)s->words[0154u+12u*free_slot+11u]=AGC_NEG_ZERO;
 if(pending_slot==free_slot&&pending_slot)pending_slot=(uint8_t)((pending_slot+1u)%8u);
 s->words[067]=(agc_word)(pending_slot*12u);
 if((n&0177u)==0100u)s->words[067]=AGC_NEG_ZERO;
}
