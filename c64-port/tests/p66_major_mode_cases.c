/* SPDX-License-Identifier: MIT */
#include "p66_major_mode_cases.h"
#include "p66_bias_cases.h"
void p66_major_mode_prepare(apollo_overlay_state *s,uint16_t n){
 uint8_t i,slot=(uint8_t)(1u+n%8u);p66_bias_prepare(s,n);
 s->words[1]=01400;s->words[2]=02501;s->words[5]=05311;s->words[0167]=(uint16_t)(035000+s->words[0120]);
 for(i=1;i<8;++i)s->words[0154+i*12u+11u]=i==slot ? 077777 : 04110;
}
