/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_verb_dispatch_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status;for(n=0;n<DSKY_VERB_DISPATCH_CASES;++n){p66_bias_io io;enum dsky_verb_route route=DSKY_VERB_DIRECT;dsky_verb_dispatch_input(&s,&io,n);if(n<4096u)status=(uint16_t)dsky_verb_dispatch(&s,&io,&route);else{cm_native_io p={0};p.superbank=io.superbank;p.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)cm_verb_dispatch(&s,&p,&route);io.superbank=p.superbank;io.interrupt_enabled=p.interrupt_enabled;}printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);}return 0;}
