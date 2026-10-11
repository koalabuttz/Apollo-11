/* SPDX-License-Identifier: MIT */
#include "lm_delay_image_cases.h"
#include "lm_waitlist_image_cases.h"
agc_word lm_delay_image_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 agc_word delay;lm_waitlist_image_input(s,io,n);delay=agc_correct(s->words[0]);
 s->words[5]=n&1u?05224:05221;io->interrupt_enabled=0;
 s->words[0705]=delay;
 if(n%3u==2u){s->words[5]=03231;s->words[0]=agc_extend(s->words[6]);
  s->words[1]=agc_extend((agc_word)(s->words[6]|io->superbank));
  s->words[2]=agc_extend(delay);s->words[061]=n&4u?015257:03210;
  s->words[063]=03263;s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;}
 return delay;
}
