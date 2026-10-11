/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_launch_cases.h"
int main(void){uint16_t n,i;static apollo_overlay_state s;for(n=0;n<P66_LAUNCH_CASE_COUNT;++n){p66_bias_io io={(uint16_t)((n/5u)%4u*020u),(uint16_t)(n%2u)};p66_launch_prepare(&s,n);printf("%u %u",n,(unsigned)p66_launch(&s,&io));for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
