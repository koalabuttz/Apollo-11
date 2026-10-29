/* SPDX-License-Identifier: MIT
 * Independent source inputs only, no native guidance linked. */
#include <stdio.h>
#include "dsky_scale_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_SCALE_CASE_COUNT;++n){dsky_scale_prepare(&s,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" 64 %u\n",n%2u);}return 0;}
