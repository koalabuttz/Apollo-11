/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "sband_control_cases.h"
int main(int argc,char **argv){int j;for(j=1;j<argc;++j){uint32_t n=(uint32_t)strtoul(argv[j],0,0);apollo_overlay_state m;p66_bias_io io;enum apollo_fixed_mission mission;enum sband_control_entry entry;uint16_t row[2050];unsigned i;sband_control_input(&m,&io,n,&mission,&entry);for(i=0;i<2048u;++i)row[i]=m.words[i];row[2048]=io.interrupt_enabled;row[2049]=io.superbank;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
