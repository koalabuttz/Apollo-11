/* SPDX-License-Identifier: MIT
 * Independent initial state constructors, no native display linked. */
#include <stdio.h>
#include "dsky_relay_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_RELAY_CASE_COUNT;++n){dsky_relay_prepare(&s,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u 0 0\n",(n/5u)%8u*020u,n%2u);}return 0;}
