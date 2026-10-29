/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_bridge_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<THROTTLE_BRIDGE_CASES;++n){p66_bias_io io;enum p66_bridge_status status;throttle_bridge_input(&s,&io,n);status=n<4096u?p66_throttle_enter(&s,&io):p66_throttle_return(&s,&io);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
