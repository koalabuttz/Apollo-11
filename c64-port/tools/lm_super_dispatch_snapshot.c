/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_super_dispatch_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_SUPER_DISPATCH_CASES;++n){apollo_overlay_state m;p66_bias_io io;exec_transfer next;unsigned i;enum exec_status st;lm_super_dispatch_input(&m,&io,n);st=lm_super_dispatch(&m,&io,&next);printf("%u %u",n,(unsigned)st);for(i=0;i<2048u;++i)printf(" %u",m.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
