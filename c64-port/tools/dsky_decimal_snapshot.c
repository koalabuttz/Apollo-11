/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_decimal_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<DSKY_DECIMAL_CASE_COUNT;++n){p66_bias_io io={0100,(uint16_t)(n%2u)};dsky_decimal_prepare(&s,n);printf("%u %u",n,(unsigned)dsky_decimal_display(&s,&io,(uint8_t)(n%2u)));for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
