/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_bridge_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<THROTTLE_BRIDGE_CASES;++n){p66_bias_io io;throttle_bridge_input(&s,&io,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
