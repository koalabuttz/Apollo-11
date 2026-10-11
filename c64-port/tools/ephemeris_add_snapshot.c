/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "ephemeris_add_cases.h"
int main(int argc,char **argv){uint32_t n,limit=argc>2?(uint32_t)strtoul(argv[2],0,0):65536u;enum apollo_fixed_mission mission=argc>1&&argv[1][0]=='C'?APOLLO_FIXED_CM:APOLLO_FIXED_LM;
 for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq,super;uint16_t row[2053];unsigned i;ephemeris_add_input(&m,&irq,&super,n,mission);row[4]=ephemeris_triple_add(&m,mission);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=irq;row[3]=super;for(i=0;i<2048u;++i)row[i+5u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
