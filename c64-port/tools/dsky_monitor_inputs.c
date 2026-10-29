/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "dsky_monitor_cases.h"
static apollo_overlay_state s;
int main(int argc,char **argv){int a;unsigned i;enum dsky_nv_mission mission;enum dsky_monitor_stage stage;dsky_charin_io io;for(a=1;a<argc;++a){uint16_t n=(uint16_t)strtoul(argv[a],0,10);dsky_monitor_input(&s,&io,n,&mission,&stage);printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.control.superbank,io.control.interrupt_enabled,io.channel11,io.wrote_channel11);}return 0;}
