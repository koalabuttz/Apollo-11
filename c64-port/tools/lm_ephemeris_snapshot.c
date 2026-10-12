/* SPDX-License-Identifier: MIT */
/* Native LSPOS/LUNVEL producer in the reference row format:
 * n lo, n hi, IRQ, SUPER, status, then the 2048-word image. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_ephemeris.h"
#include "lm_ephemeris_cases.h"
int main(int argc,char **argv){uint32_t n,limit=argc>1?(uint32_t)strtoul(argv[1],0,0):65536u;
 for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;uint16_t row[2053];unsigned i;uint8_t status;
  lm_ephemeris_input(&m,&io,n);status=lm_ephemeris(&m,&io);
  row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;row[4]=status;
  for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)m.words[i];
  if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
 return 0;}
