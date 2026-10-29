/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_priority_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<LM_PRIORITY_CASES;++n){p66_bias_io io;exec_address next;enum exec_status status;lm_priority_input(&s,&io,n);status=lm_basic_priority_change(&s,&io,&next);printf("%u %u",n,(unsigned)status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
