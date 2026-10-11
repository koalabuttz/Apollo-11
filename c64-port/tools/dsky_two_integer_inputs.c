/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_two_integer_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;enum dsky_nv_mission mission;p66_bias_io io;for(n=0;n<DSKY_TWO_INTEGER_CASES;++n){dsky_two_integer_input(&s,&io,n,&mission);printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
