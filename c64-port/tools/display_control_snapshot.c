/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "display_control_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<DISPLAY_CONTROL_CASES;++n){p66_bias_io io;enum p66_display_boundary status;display_control_input(&s,&io,n);status=p66_display_exit(&s,&io);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
