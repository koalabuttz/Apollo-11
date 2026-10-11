/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "sband_frame_cases.h"
int main(int argc,char **argv){uint32_t n,start=argc>1?(uint32_t)strtoul(argv[1],0,0):0,end=argc>2?(uint32_t)strtoul(argv[2],0,0):49152;
 for(n=start;n<end;++n){apollo_overlay_state m;p66_bias_io io;enum apollo_fixed_mission mission;enum sband_frame_stage entry;uint16_t row[2053];unsigned i;
  sband_frame_input(&m,&io,n,&mission,&entry);
  row[4]=(uint16_t)sband_frame(&m,&io,mission,entry);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;
  for(i=0;i<2048u;++i){row[i+5u]=m.words[i];}if(fwrite(row,sizeof row,1,stdout)!=1)return 2;
 }return 0;
}
