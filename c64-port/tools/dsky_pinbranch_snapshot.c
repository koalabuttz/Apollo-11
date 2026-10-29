/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_pinbranch_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status;for(n=0;n<DSKY_PINBRANCH_CASES;++n){p66_bias_io io;enum dsky_pinbranch_route route=DSKY_PINBRANCH_ERASE;dsky_pinbranch_input(&s,&io,n);if(n<8192u)status=(uint16_t)(((n/4096u)%2u) ? dsky_pinbranch_resume(&s,&io,&route) : dsky_pinbranch(&s,&io,&route));else{cm_native_io p={0};p.superbank=io.superbank;p.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)(((n/4096u)%2u) ? cm_pinbranch_resume(&s,&p,&route) : cm_pinbranch(&s,&p,&route));io.superbank=p.superbank;io.interrupt_enabled=p.interrupt_enabled;}printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);}return 0;}
