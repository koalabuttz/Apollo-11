/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "runtime_bridge_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<RUNTIME_BRIDGE_CASES;++n){p66_bias_io io;uint16_t status;runtime_bridge_input(&s,&io,n);status=runtime_bridge_apply(&s,&io,n);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
