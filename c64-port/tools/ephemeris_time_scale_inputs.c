/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "ephemeris_time_scale_cases.h"
int main(int argc,char **argv){int j;enum apollo_fixed_mission mission;if(argc<3)return 2;mission=argv[1][0]=='C'?APOLLO_FIXED_CM:APOLLO_FIXED_LM;
 for(j=2;j<argc;++j){apollo_overlay_state m;agc_word irq,super;uint16_t row[2050];unsigned i;ephemeris_time_scale_input(&m,&irq,&super,(uint32_t)strtoul(argv[j],0,0),mission);for(i=0;i<2048u;++i)row[i]=m.words[i];row[2048]=irq;row[2049]=super;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
