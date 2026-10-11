/* SPDX-License-Identifier: MIT */
#include "runtime_fetch_cases.h"
#include "runtime_bridge_cases.h"
void runtime_fetch_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint16_t k=(uint16_t)(n&4095u);runtime_bridge_input(s,io,k);
 if(n<4096u){s->words[5]=06067;s->words[0164]=03507;s->words[020]=020006;}
 else{s->words[5]=06070;s->words[0164]=03510;s->words[020]=(uint16_t)(((uint32_t)k*37u)&077777u);}
}
