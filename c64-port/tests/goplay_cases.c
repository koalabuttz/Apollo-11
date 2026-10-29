/* SPDX-License-Identifier: MIT */
#include "goplay_cases.h"
#include "throttle_bridge_cases.h"
void goplay_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n,uint8_t phase){throttle_bridge_input(s,io,n);s->words[3]=03400;s->words[4]=020000;s->words[6]=020007;s->words[5]=phase?03122:03070;s->words[0164]=2;s->words[01072]=023400;s->words[0371]=01474;io->superbank=(uint16_t)((n%4u)*020u);io->interrupt_enabled=(uint16_t)(n%2u);s->words[01012]=0;s->words[01021]&=057777u;*channel=(agc_word)(n*977u&AGC_MASK);}
