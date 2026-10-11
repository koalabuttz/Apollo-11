/* SPDX-License-Identifier: MIT
 * Independent source initial constructors, no native services linked. */
#include <stdio.h>
#include "dsky_key_irq_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_KEY_IRQ_CASE_COUNT;++n){dsky_key_irq_prepare(&s,n);printf("%u 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u 0 1\n",(unsigned)((n/512u)%8u*020u));}return 0;}
