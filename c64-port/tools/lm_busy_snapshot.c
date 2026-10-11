/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_busy_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<LM_BUSY_CASES;++n){p66_bias_io io;agc_word channel;enum lm_busy_status status;lm_busy_input(&s,&io,&channel,n);status=lm_pinball_busy(&s,&io,&channel);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);}return 0;}
