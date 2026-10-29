/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_lamp_test_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,status=0;for(n=0;n<DSKY_LAMP_TEST_CASES;++n){dsky_lamp_io io;enum dsky_lamp_route route=DSKY_LAMP_WAIT;dsky_lamp_test_input(&s,&io,n);printf("%u %u",n,status);for(i=0;i<2048;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.channel12,io.channel13,io.wrote_channels,io.restart_light,(unsigned)route);}return 0;}
