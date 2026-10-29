/* SPDX-License-Identifier: MIT
 * Independent source initial constructors, no native services linked. */
#include <stdio.h>
#include "dsky_keycom_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_KEYCOM_CASE_COUNT;++n){dsky_keycom_prepare(&s,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",(unsigned)((n/512u)%8u*020u),n%2u);}return 0;}
