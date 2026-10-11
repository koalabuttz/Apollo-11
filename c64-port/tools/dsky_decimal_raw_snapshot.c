/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_decimal_raw_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;enum dsky_decimal_general_route route;uint8_t rounded;uint16_t i;uint32_t n;for(n=0;n<DSKY_DECIMAL_RAW_CASES;++n){dsky_decimal_raw_input(&s,&io,n,&mission,&rounded);if(dsky_decimal_raw(&s,&io,mission,rounded,&route)!=P66_BIAS_OK)return 2;printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);}return 0;}
