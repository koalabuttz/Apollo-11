/* SPDX-License-Identifier: MIT
 * Independent source initial constructors, no native services linked. */
#include <stdio.h>
#include "dsky_pinball_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_PINBALL_CASE_COUNT;++n){dsky_pinball_prepare(&s,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" 64 %u\n",n%2u);}return 0;}
