/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_charin_reset_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status;for(n=0;n<DSKY_CHARIN_RESET_CASES;++n){dsky_reset_io io;dsky_charin_reset_input(&s,&io,n);if(n<4096u)status=(uint16_t)dsky_charin_reset(&s,&io);else{cm_native_io p={0};p.superbank=io.control.superbank;p.interrupt_enabled=io.control.interrupt_enabled;p.channel11=io.channel11;p.channel13=io.channel13;status=(uint16_t)cm_charin_reset(&s,&p,&io.wrote_channels,&io.restart_light);io.control.superbank=p.superbank;io.control.interrupt_enabled=p.interrupt_enabled;io.channel11=p.channel11;io.channel13=p.channel13;}printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.channel13,io.wrote_channels,io.restart_light);}return 0;}
