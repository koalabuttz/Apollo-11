/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "sband_shift_cases.h"
int main(int argc,char **argv){uint32_t n,start=argc>1?(uint32_t)strtoul(argv[1],0,0):0,end=argc>2?(uint32_t)strtoul(argv[2],0,0):524288;
 for(n=start;n<end;++n){apollo_overlay_state m;agc_word irq,super;enum apollo_fixed_mission mission;uint8_t count;uint16_t row[2053];unsigned i;
  sband_shift_input(&m,&irq,&super,n,&mission,&count);row[4]=sband_shift_left(&m,mission,count);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=irq;row[3]=super;
  for(i=0;i<2048u;++i){row[i+5u]=m.words[i];}if(fwrite(row,sizeof row,1,stdout)!=1)return 2;
 }return 0;
}
