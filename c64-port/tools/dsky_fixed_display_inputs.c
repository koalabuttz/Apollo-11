/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "dsky_fixed_display_cases.h"
static apollo_overlay_state s;
int main(int argc,char **argv){int a;unsigned i;enum dsky_nv_mission mission;p66_bias_io io;for(a=1;a<argc;++a){uint32_t n=(uint32_t)strtoul(argv[a],0,10);dsky_fixed_display_input(&s,&io,n,&mission);printf("%lu",(unsigned long)n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
