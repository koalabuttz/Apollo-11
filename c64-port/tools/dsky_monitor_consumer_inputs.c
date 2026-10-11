/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_monitor_consumer_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;enum dsky_nv_mission m;dsky_charin_io io;agc_word channel;for(n=0;n<256u;++n){dsky_monitor_consumer_input(&s,&io,n,&m,&channel);printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.wrote_channel11);}return 0;}
