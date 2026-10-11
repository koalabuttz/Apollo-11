/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_decimal_load_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;uint16_t n,i;for(n=0;n<DSKY_DECIMAL_LOAD_CASES;++n){dsky_decimal_load_input(&s,&io,n,&mission);printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
