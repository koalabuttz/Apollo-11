/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_load_request_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status=0;for(n=0;n<DSKY_LOAD_REQUEST_CASES;++n){dsky_lamp_io io;enum dsky_load_request_route route=DSKY_LOAD_REQUEST_END;dsky_load_request_input(&s,&io,n);printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.channel12,io.channel13,io.wrote_channels,io.restart_light,(unsigned)route);}return 0;}
