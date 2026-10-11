/* SPDX-License-Identifier: MIT */
#include "display_control_cases.h"
#include "throttle_bridge_cases.h"
void display_control_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 throttle_bridge_input(s,io,n);s->words[3]=03400;s->words[4]=064000;s->words[6]=064007;s->words[5]=03531;s->words[0164]=03530;s->words[0165]=064007;s->words[0163]=0;s->words[020]=0;s->words[023]=0;s->words[067]=0;s->words[03621]=2;
 if(n&1u)s->words[0104]|=01000u;else s->words[0104]&=076777u;
}
