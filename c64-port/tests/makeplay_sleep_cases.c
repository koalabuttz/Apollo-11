/* SPDX-License-Identifier: MIT */
#include "makeplay_sleep_cases.h"
#include "makeplay_cases.h"
void makeplay_sleep_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){agc_word channel;uint16_t k=(uint16_t)(n&4095u);makeplay_input(s,io,&channel,k);
 s->words[5]=(agc_word)(n<4096u?02754u:n<8192u?02770u:n<12288u?02615u:03121u);s->words[0154]=1;s->words[0162]=011254;
 if(n>=8192u&&n<12288u)s->words[0164]=(agc_word)(k%8u);
 if(n>=12288u){if(k%4u==0u)s->words[01042]=0;else if(k%4u==1u)s->words[01042]=AGC_NEG_ZERO;}
}
enum p66_sleep_boundary makeplay_sleep_apply(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){if(n<4096u)return p66_makeplay_sleep_enter(s,io);if(n<8192u)return p66_makeplay_sleep_return(s,io);if(n<12288u)return p66_makeplay_sleep_resume(s,io);return p66_makeplay_busy(s,io);}
