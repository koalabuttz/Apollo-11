/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_charin_reset_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<DSKY_CHARIN_RESET_CASES;++n){dsky_reset_io io;dsky_charin_reset_input(&s,&io,n);printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.channel13,io.wrote_channels,io.restart_light);}return 0;}
