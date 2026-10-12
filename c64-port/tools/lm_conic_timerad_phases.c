/* SPDX-License-Identifier: MIT */
/* Native TIMERAD/APSIDES phase frontiers of the named sampled entries: one row per
 * frontier (n lo, n hi, IRQ, SUPER, phase about to run, 2048 words), from
 * the entry image through the phase that ends the call. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_conic_timerad.h"
#include "lm_conic_timerad_cases.h"
static int emit(uint32_t n,const apollo_overlay_state *m,const p66_bias_io *io,uint8_t phase){uint16_t row[2053];unsigned i;
 row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io->interrupt_enabled;row[3]=io->superbank;row[4]=phase;
 for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)m->words[i];
 row[5u+2u]&=077777u;
 return fwrite(row,sizeof row,1,stdout)==1;}
int main(int argc,char **argv){int j;
 for(j=1;j<argc;++j){apollo_overlay_state m;p66_bias_io io;uint32_t n=(uint32_t)strtoul(argv[j],0,0);uint8_t phase;unsigned guard;
  lm_conic_timerad_input(&m,&io,n);phase=m.words[0164]==03547u?LTR_TIMERAD:LTR_APSIDES;if(!emit(n,&m,&io,phase))return 2;
  for(guard=0;guard<200u&&!LM_CONIC_TIMERAD_DONE(phase);++guard){phase=lm_conic_timerad_step(&m,&io,phase);if(!emit(n,&m,&io,phase))return 2;}
  if(phase!=LTT_RETURNED&&phase!=LTT_ABORTED&&phase!=LTT_CONICABORT)return 3;}
 return 0;}
