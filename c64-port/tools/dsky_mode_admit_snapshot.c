/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_mode_admit_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status=0;for(n=0;n<DSKY_MODE_ADMIT_CASES;++n){p66_bias_io io;uint16_t alarm=0;enum dsky_mode_admit_route route=DSKY_MODE_ADMIT_CALLER;dsky_mode_admit_input(&s,&io,n);if(n<4096u)status=(uint16_t)dsky_mode_admit(&s,&io,&route);else{cm_native_io p={0};p.superbank=io.superbank;p.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)cm_mode_admit(&s,&p,&route);io.superbank=p.superbank;io.interrupt_enabled=p.interrupt_enabled;alarm=p.executive_alarm;}printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,alarm,(unsigned)route);}return 0;}
