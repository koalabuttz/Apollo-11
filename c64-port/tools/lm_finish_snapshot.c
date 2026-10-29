/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_finish_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<LM_FINISH_CASES;++n){p66_bias_io io;agc_word channel;exec_transfer next;enum exec_status status;lm_finish_input(&s,&io,&channel,n);status=lm_basic_finish(&s,&io,&channel,&next);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);}return 0;}
