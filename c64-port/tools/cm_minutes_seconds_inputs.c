/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_minutes_seconds_cases.h"
#include "cm_decimal_pair_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;for(n=0;n<1536u;++n){if(n<1024u)cm_decimal_pair_input(&s,&io,n);else cm_minutes_seconds_input(&s,&io,(uint16_t)(n-1024u));printf("%u 1",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
