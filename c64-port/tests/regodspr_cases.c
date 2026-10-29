/* SPDX-License-Identifier: MIT */
#include "regodspr_cases.h"
#include "display_control_cases.h"
void regodspr_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint8_t i,slot=(uint8_t)(n%8u);display_control_input(s,io,(uint16_t)(n&077776u));
 s->words[0]=01474;s->words[2]=04631;s->words[3]=03400;s->words[4]=020000;s->words[5]=02470;s->words[6]=020007;s->words[0133]=03476;s->words[0134]=062000;
 s->words[0167]=(agc_word)(((n/8u)%31u+1u)*01000u+0401u);
 for(i=1;i<8u;++i)s->words[0154u+12u*i+11u]=(agc_word)(((n*17u+i)%31u+1u)*01000u+0110u);
 if(slot)s->words[0154u+12u*slot+11u]=AGC_NEG_ZERO;
 if(slot&&n&0100u)for(i=(uint8_t)(slot+1u);i<8u;++i)s->words[0154u+12u*i+11u]=AGC_NEG_ZERO;
}
