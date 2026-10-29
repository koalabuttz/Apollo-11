/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "sband_shift_cases.h"
int main(int argc,char **argv){int j;for(j=1;j<argc;++j){uint32_t n=(uint32_t)strtoul(argv[j],0,0);apollo_overlay_state m;agc_word irq,super;uint8_t count;enum apollo_fixed_mission mission;uint16_t row[2050];unsigned i;sband_shift_input(&m,&irq,&super,n,&mission,&count);for(i=0;i<2048u;++i)row[i]=m.words[i];row[2048]=irq;row[2049]=super;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
