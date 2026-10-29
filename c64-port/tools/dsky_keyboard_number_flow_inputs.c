/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_keyboard_number_flow_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_KEYBOARD_NUMBER_FLOW_CASE_COUNT;++n){dsky_keyboard_number_flow_prepare(&s,n);printf("%u 0 0",n);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u 1 %u\n",(unsigned)((n/128u)%8u*020u),dsky_keyboard_number_flow_channel11(n));}return 0;}
