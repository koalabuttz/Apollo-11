/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_decimal_select_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;uint16_t n,i;agc_word route;for(n=0;n<DSKY_DECIMAL_SELECT_CASES;++n){dsky_decimal_select_input(&s,&io,n,&mission);if(dsky_decimal_select(&s,&io,mission,&route)!=P66_BIAS_OK)return 2;printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
