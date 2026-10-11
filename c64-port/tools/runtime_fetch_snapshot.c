/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "runtime_fetch_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<RUNTIME_FETCH_CASES;++n){p66_bias_io io;enum p66_bridge_status status;runtime_fetch_input(&s,&io,n);status=p66_runtime_throttle_fetch(&s,&io);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
