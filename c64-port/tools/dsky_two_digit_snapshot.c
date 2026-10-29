/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_two_digit_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_TWO_DIGIT_CASE_COUNT;++n){p66_bias_io io={0100,(uint16_t)(n%2u)};dsky_two_digit_prepare(&s,n);printf("%u %u",n,(unsigned)dsky_two_digit(&s,&io));for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
