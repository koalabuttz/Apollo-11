/* SPDX-License-Identifier: MIT */
#include "nv_entry_cases.h"
#include "throttle_bridge_cases.h"
void nv_entry_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 throttle_bridge_input(s,io,n);s->words[0]=01474;s->words[1]=0;s->words[2]=03121;s->words[3]=03400;s->words[4]=020000;s->words[5]=04155;s->words[6]=020007;
 s->words[01012]=(uint16_t)((n&1u)?((n>>1)%7u+1u):0u);
 if(n&010u)s->words[01021]|=020000u;else s->words[01021]&=057777u;
}
