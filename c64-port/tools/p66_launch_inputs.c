/* SPDX-License-Identifier: MIT
 * Initial state constructors only, no native guidance linked. */
#include <stdio.h>
#include "p66_launch_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<P66_LAUNCH_CASE_COUNT;++n){p66_launch_prepare(&s,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",(n/5u)%4u*020u,n%2u);}return 0;}
