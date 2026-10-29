/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "makeplay_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<MAKEPLAY_CASES;++n){p66_bias_io io;agc_word channel;enum p66_makeplay_boundary result;makeplay_input(&s,&io,&channel,n);result=makeplay_apply(&s,&io,&channel,n);printf("%u %u",n,(unsigned)result);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);}return 0;}
