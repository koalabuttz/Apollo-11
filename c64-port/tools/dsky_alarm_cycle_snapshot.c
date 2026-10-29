/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_alarm_cycle_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status;for(n=0;n<DSKY_ALARM_CYCLE_CASES;++n){dsky_charin_io io;enum dsky_alarm_cycle_route route=DSKY_ALARM_CYCLE_UPDATE;dsky_alarm_cycle_input(&s,&io,n);if(n<4096u)status=(uint16_t)dsky_alarm_cycle(&s,&io,&route);else{cm_native_io p={0};p.superbank=io.control.superbank;p.interrupt_enabled=io.control.interrupt_enabled;p.channel11=io.channel11;status=(uint16_t)cm_alarm_cycle(&s,&p,&io.wrote_channel11,&route);io.control.superbank=p.superbank;io.control.interrupt_enabled=p.interrupt_enabled;io.channel11=p.channel11;}printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.wrote_channel11,(unsigned)route);}return 0;}
