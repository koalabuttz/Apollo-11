/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_monitor_paste_cases.h"
static apollo_overlay_state s;
int main(void){uint32_t n;uint16_t i;enum dsky_nv_mission mission;enum dsky_monitor_paste_stage stage;p66_bias_io io;for(n=0;n<DSKY_MONITOR_PASTE_CASES;++n){dsky_monitor_paste_input(&s,&io,(uint16_t)n,&mission,&stage);if(dsky_monitor_paste(&s,&io,mission,stage)!=P66_BIAS_OK)return 2;printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
