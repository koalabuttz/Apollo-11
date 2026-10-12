/* SPDX-License-Identifier: MIT */
/* Native TIME_OF_FREE_FALL producer in the reference row format: n lo, n hi,
 * IRQ, SUPER, status, then the 2048-word image after the sampled call chain
 * (status 1 returned from every call, 2 SQRT abort, 0 unsupported). */
#include <stdio.h>
#include <stdlib.h>
#include "lm_time_free_fall.h"
#include "lm_time_free_fall_cases.h"
int main(int argc,char **argv){uint32_t n,limit=argc>1?(uint32_t)strtoul(argv[1],0,0):65536u;
 for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;uint16_t row[2053];unsigned i;uint8_t status=1,k,calls=lm_time_free_fall_calls(n);
  lm_time_free_fall_input(&m,&io,n);
  for(k=0;k<calls&&status==1u;++k){if(k)lm_time_free_fall_call(&m,n,k);status=lm_time_free_fall(&m,&io);}
  row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;row[4]=status;
  for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)m.words[i];
  row[5u+2u]&=077777u;
  if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
 return 0;}
