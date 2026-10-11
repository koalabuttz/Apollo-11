/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_blanksub_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status;for(n=0;n<DSKY_BLANKSUB_CASES;++n){p66_bias_io io;dsky_blanksub_input(&s,&io,n);if(n<4096u)status=(uint16_t)dsky_blanksub(&s,&io);else{cm_native_io p={0};p.superbank=io.superbank;p.interrupt_enabled=io.interrupt_enabled;status=(uint16_t)cm_blanksub(&s,&p);io.superbank=p.superbank;io.interrupt_enabled=p.interrupt_enabled;}printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
