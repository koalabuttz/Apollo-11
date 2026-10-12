/* SPDX-License-Identifier: MIT */
/* Native LSPOS/LUNVEL phase frontiers of the named sampled states: one row per
 * frontier (n lo, n hi, IRQ, SUPER, phase about to run, 2048 words), from the
 * entry image through the phase that ends the routine. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_ephemeris.h"
#include "lm_ephemeris_cases.h"
static int emit(uint32_t n,const apollo_overlay_state *m,const p66_bias_io *io,uint8_t phase){uint16_t row[2053];unsigned i;
 row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io->interrupt_enabled;row[3]=io->superbank;row[4]=phase;
 for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)m->words[i];
 return fwrite(row,sizeof row,1,stdout)==1;}
int main(int argc,char **argv){int j;
 for(j=1;j<argc;++j){apollo_overlay_state m;p66_bias_io io;uint32_t n=(uint32_t)strtoul(argv[j],0,0);uint8_t phase=LME_ENTRY;unsigned guard;
  lm_ephemeris_input(&m,&io,n);if(!emit(n,&m,&io,phase))return 2;
  for(guard=0;guard<64u&&phase<LME_RETURNED;++guard){phase=lm_ephemeris_step(&m,&io,phase);if(!emit(n,&m,&io,phase))return 2;}
  if(phase!=LME_RETURNED)return 3;}
 return 0;}
