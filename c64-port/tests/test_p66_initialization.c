/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_initialization_check.h"
int main(void){uint8_t n;uint16_t failures=0;p66_init_case s,before;for(n=0;n<P66_INIT_TARGET_COUNT;++n)failures=(uint16_t)(failures+p66_init_check(n,&s));p66_init_prepare(&s,1);s.state.superbank=0100;before=s;if(p66_fastchng(&s.state)!=P66_INIT_OUT_OF_SCOPE || memcmp(&s,&before,sizeof s))++failures;p66_init_prepare(&s,0);s.state.q=3;before=s;if(p66_stoprate(&s.state)!=P66_INIT_OUT_OF_SCOPE || memcmp(&s,&before,sizeof s))++failures;printf("P66 INITIALIZATION: 128 FULL STATES, %u FAILURES\n",failures);return failures!=0;}
