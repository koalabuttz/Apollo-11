/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_monitor_cases.h"
static apollo_overlay_state s;
int main(void){uint32_t n;uint16_t i;enum dsky_nv_mission mission;enum dsky_monitor_stage stage;enum dsky_monitor_route route;dsky_charin_io io;for(n=0;n<DSKY_MONITOR_CASES;++n){dsky_monitor_input(&s,&io,(uint16_t)n,&mission,&stage);if(dsky_monitor(&s,&io,mission,stage,&route)!=P66_BIAS_OK)return 2;printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.wrote_channel11,(unsigned)route);}return 0;}
