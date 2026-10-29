/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_test_noun_cases.h"
static apollo_overlay_state s;
int main(void){uint32_t n;uint16_t i;enum dsky_nv_mission mission;enum dsky_enter_zero_route route;p66_bias_io io;for(n=0;n<DSKY_TEST_NOUN_CASES;++n){dsky_test_noun_input(&s,&io,(uint16_t)n,&mission);if(dsky_test_noun(&s,&io,mission,&route)!=P66_BIAS_OK)return 2;printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);}return 0;}
