/* SPDX-License-Identifier: MIT */
#include "runtime_bridge_cases.h"
#include "throttle_bridge_cases.h"
#include "display_control_cases.h"
void runtime_bridge_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint16_t k=(uint16_t)(n&4095u);
 if(n<8192u){throttle_bridge_input(s,io,k);s->words[5]=(uint16_t)(n<4096u?06070u:06743u);s->words[0164]=03510;s->words[020]=0;}
 else if(n<12288u)throttle_bridge_input(s,io,(uint16_t)(k+4096u));
 else display_control_input(s,io,k);
 s->words[067]=(uint16_t)(((uint32_t)k*13u)&077777u);
 if(k<8u){static const uint16_t pending[8]={0,1,12,24,84,037777,040000,077777};s->words[067]=pending[k];}
}
uint16_t runtime_bridge_apply(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 if(n<8192u)return (uint16_t)p66_runtime_throttle_enter(s,io);
 if(n<12288u)return (uint16_t)p66_runtime_throttle_return(s,io);
 return (uint16_t)p66_runtime_display_exit(s,io);
}
