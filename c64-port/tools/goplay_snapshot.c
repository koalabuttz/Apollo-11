/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "goplay_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;uint8_t phase;for(n=0;n<GOPLAY_CASES;++n)for(phase=0;phase<2u;++phase){p66_bias_io io;agc_word channel;unsigned status;goplay_input(&s,&io,&channel,n,phase);status=(unsigned)(phase?p66_goplay_finish(&s,&io,&channel):p66_goplay_enter(&s,&io));printf("%u %u %u",n,phase,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);}return 0;}
