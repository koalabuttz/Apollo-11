/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only original STRTP66A oracle, no native initialization linked. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "p66_bias_cases.h"
int main(int argc,char **argv){
 unsigned n,i,cycles,pc;static agc_t pristine;static apollo_overlay_state state;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 for(n=0;n<P66_BIAS_CASE_COUNT;++n){
  memcpy(&machine,&pristine,sizeof machine);p66_bias_prepare(&state,(uint16_t)n);InhibitAlarms=1;
  for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)state.words[i];
  machine.OutputChannel7=machine.InputChannel[7]=(int)((n/5u)%4u*020u);machine.AllowInterrupt=n%2u;
  for(cycles=0;cycles<15000;++cycles){pc=get(RegZ);if(!machine.PendFlag && !machine.ExtraDelay && pc==02536 && get(RegFB)==062000)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==15000){fprintf(stderr,"bias%u timeout PC%05o LOC%05o BB%05o\n",n,pc,get(0164),get(RegBB));return 2;}
  printf("%u 0",n);
  for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2 ? 0177777u : 077777u));
  printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
  fprintf(stderr,"bias%u %ucycles PC%05o BB%05o\n",n,cycles,pc,get(RegBB));
 }
 return 0;
}
