/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_clock_display_mission_cases.h"
static apollo_overlay_state s;
int main(void){static const uint16_t selected[9]={0,2,3,6,7,10,11,14,15};cm_native_io io;uint16_t n,i;for(n=0;n<9u;++n){cm_clock_display_input(&s,&io,selected[n]);printf("%u",selected[n]);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u 0 0 0 0 0\n",io.superbank,io.interrupt_enabled,io.channel11);}return 0;}
