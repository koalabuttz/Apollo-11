/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_major_request_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<DSKY_MAJOR_REQUEST_CASES;++n){dsky_charin_io io;dsky_major_request_input(&s,&io,n);printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.wrote_channel11,0u);}return 0;}
