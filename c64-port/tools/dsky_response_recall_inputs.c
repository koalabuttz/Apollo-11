/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_response_recall_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status=0;for(n=0;n<DSKY_RESPONSE_RECALL_CASES;++n){dsky_lamp_io io;dsky_response_recall_input(&s,&io,n);printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.channel12,io.channel13,io.wrote_channels,io.restart_light,0u);}return 0;}
