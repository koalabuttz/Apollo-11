/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "makeplay_sleep_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<MAKEPLAY_SLEEP_CASES;++n){p66_bias_io io;uint16_t status;makeplay_sleep_input(&s,&io,n);status=makeplay_sleep_apply(&s,&io,n);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
