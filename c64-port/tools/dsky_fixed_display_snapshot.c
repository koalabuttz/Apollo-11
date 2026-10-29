/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_fixed_display_cases.h"
static apollo_overlay_state s;
int main(void){uint32_t n;unsigned stage,i;uint16_t address;agc_word value;enum dsky_nv_mission mission;enum dsky_fixed_display_route route;p66_bias_io io;for(n=0;n<DSKY_FIXED_DISPLAY_CASES;++n){dsky_fixed_display_input(&s,&io,n,&mission);for(stage=0;stage<3u;++stage){value=0;if(stage==1u&&(!dsky_fixed_display_address(&s,&io,mission,&address)||!dsky_fixed_display_data(mission,address,&value,0)))return 2;if(dsky_fixed_display(&s,&io,mission,(enum dsky_fixed_display_stage)stage,value,&route)!=P66_BIAS_OK)return 2;printf("%lu",(unsigned long)(n*3u+stage));for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);}}return 0;}
