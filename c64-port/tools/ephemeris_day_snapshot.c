/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "ephemeris_day_cases.h"
int main(int argc,char **argv){uint32_t n,limit=65536u;enum apollo_fixed_mission mission;if(argc<2)return 2;mission=argv[1][0]=='C'?APOLLO_FIXED_CM:APOLLO_FIXED_LM;if(argc>2)limit=(uint32_t)strtoul(argv[2],0,0);for(n=0;n<limit;++n){apollo_overlay_state m;uint16_t irq,super,row[2053];unsigned i;ephemeris_day_input(&m,&irq,&super,n,mission);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=irq;row[3]=super;row[4]=ephemeris_day_divide(&m,mission);for(i=0;i<2048u;++i)row[i+5u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
