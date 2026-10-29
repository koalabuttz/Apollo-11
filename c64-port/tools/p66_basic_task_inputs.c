/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_basic_task_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<P66_BASIC_TASK_CASE_COUNT;++n){p66_basic_task_prepare(&s,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",(unsigned)((n/432u)%4u*020u),(unsigned)(n%2u));}return 0;}
