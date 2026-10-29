/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "regodspr_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<REGODSPR_CASES;++n){p66_bias_io io;enum exec_status result;regodspr_input(&s,&io,n);result=p66_regodspr(&s,&io);printf("%u %u",n,(unsigned)result);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
