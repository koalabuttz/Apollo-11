/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_sign_output_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;uint8_t entry;uint16_t n,i;enum dsky_sign_output_route route;for(n=0;n<DSKY_SIGN_OUTPUT_CASES;++n){dsky_sign_output_input(&s,&io,n,&mission,&entry);if(dsky_sign_output(&s,&io,mission,entry,&route)!=P66_BIAS_OK)return 2;printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);}return 0;}
