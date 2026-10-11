/* SPDX-License-Identifier: MIT */
#include "makeplay_cases.h"
#include "throttle_bridge_cases.h"
void makeplay_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){uint16_t k=(uint16_t)(n&4095u);
 throttle_bridge_input(s,io,k);s->words[3]=03400;s->words[4]=020000;s->words[6]=020007;s->words[7]=0;s->words[0155]=01474;s->words[0157]=063476;s->words[0160]=023400;s->words[0167]=(agc_word)(((k%31u)+1u)*01000u+(k&0777u));s->words[0163]=(agc_word)(s->words[0167]&037000u);s->words[0164]=2;s->words[01072]=023400;s->words[0371]=01474;s->words[01012]=(agc_word)(k%8u);
 s->words[0100]=(agc_word)(k%3u==0?s->words[0100]:k%3u==1u?s->words[0100]&011256u:(k&7u)*0100u);
 if(k%5u==0u)s->words[0374]=063476;
 s->words[5]=(agc_word)(n<4096u?02546u:n<8192u?02553u:02614u);s->words[064]=(agc_word)(k%8u?12u*(k%8u):077776u);
 io->superbank=(uint16_t)((k%8u)*020u);io->interrupt_enabled=(uint16_t)(k%2u);*channel=(agc_word)(((uint32_t)k*977u)&AGC_MASK);
}
enum p66_makeplay_boundary makeplay_apply(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){if(n<4096u)return p66_makeplay_priority(s,io);if(n<8192u)return p66_makeplay_select(s,io,channel);return p66_makeplay_wake_return(s,io);}
