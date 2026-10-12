/* SPDX-License-Identifier: MIT */
/* Native lm-r31 producer in the reference row format: n lo, n hi, IRQ,
 * SUPER, tag (1 stopped at an unmapped point, 3 anything else), then the
 * 2048-word image after the run. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_r31_cases.h"
int main(int argc,char **argv){uint32_t n,first=argc>2?(uint32_t)strtoul(argv[2],0,0):0u,limit=argc>1?(uint32_t)strtoul(argv[1],0,0):LM_R31_CASES;
 for(n=first;n<limit;++n){apollo_overlay_state m;p66_bias_io io;uint16_t row[2053];unsigned i,kind;
  kind=lm_r31_run(&m,&io,n);if(kind!=LM_R31_STOPPED)kind=3u;
  row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;row[4]=(uint16_t)kind;
  for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)m.words[i];
  row[5u+2u]&=077777u;
  if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
 return 0;}
