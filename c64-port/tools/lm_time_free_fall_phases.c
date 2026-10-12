/* SPDX-License-Identifier: MIT */
/* Native TIME_OF_FREE_FALL phase frontiers of the named sampled call chains:
 * one row per frontier (n lo, n hi, IRQ, SUPER, phase about to run, 2048
 * words), each call from its entry image through the phase that ends it. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_time_free_fall.h"
#include "lm_time_free_fall_cases.h"
static int emit(uint32_t n,const apollo_overlay_state *m,const p66_bias_io *io,uint8_t phase){uint16_t row[2053];unsigned i;
 row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io->interrupt_enabled;row[3]=io->superbank;row[4]=phase;
 for(i=0;i<2048u;++i)row[i+5u]=(uint16_t)m->words[i];
 row[5u+2u]&=077777u;
 return fwrite(row,sizeof row,1,stdout)==1;}
static uint8_t first(const apollo_overlay_state *m){
 switch(m->words[0164]){case 03360u:return LTF_ENTRY;case 03361u:return LTF_CONMU;case 03427u:return LTF_RPRA;case 03465u:return LTF_CALCTPER;default:return LTF_CALCTFF;}
}
int main(int argc,char **argv){int j;
 for(j=1;j<argc;++j){apollo_overlay_state m;p66_bias_io io;uint32_t n=(uint32_t)strtoul(argv[j],0,0);uint8_t k,calls=lm_time_free_fall_calls(n),phase=LTF_RETURNED;unsigned guard;
  lm_time_free_fall_input(&m,&io,n);
  for(k=0;k<calls&&phase==LTF_RETURNED;++k){
   if(k)lm_time_free_fall_call(&m,n,k);
   phase=first(&m);if(!emit(n,&m,&io,phase))return 2;
   for(guard=0;guard<64u&&phase<LTF_RETURNED;++guard){phase=lm_time_free_fall_step(&m,&io,phase);if(!emit(n,&m,&io,phase))return 2;}
   if(phase!=LTF_RETURNED&&phase!=LTF_ABORTED)return 3;}}
 return 0;}
