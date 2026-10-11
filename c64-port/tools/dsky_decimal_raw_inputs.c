/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_decimal_raw_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;uint8_t entry;uint16_t i;uint32_t n;for(n=0;n<DSKY_DECIMAL_RAW_CASES;++n){dsky_decimal_raw_input(&s,&io,n,&mission,&entry);printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u 2\n",io.superbank,io.interrupt_enabled);}return 0;}
