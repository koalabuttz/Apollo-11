/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_verb_dispatch_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<DSKY_VERB_DISPATCH_CASES;++n){p66_bias_io io;dsky_verb_dispatch_input(&s,&io,n);printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,0u);}return 0;}
