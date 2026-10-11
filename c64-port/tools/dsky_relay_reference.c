/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only independent original DSPOUTSB oracle, no native service linked. */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "reference_runner.c"
#undef ChannelOutput
#undef main
#include "dsky_relay_cases.h"
static unsigned wrote,channel10;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==010){wrote=1;channel10=(unsigned)v&077777u;}}
int main(int argc,char **argv){
 unsigned n,i,cycles,pc;static agc_t pristine;static apollo_overlay_state state;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 for(n=0;n<DSKY_RELAY_CASE_COUNT;++n){
  memcpy(&machine,&pristine,sizeof machine);dsky_relay_prepare(&state,(uint16_t)n);InhibitAlarms=1;wrote=channel10=0;
  for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)state.words[i];
  machine.OutputChannel7=machine.InputChannel[7]=(int)((n/5u)%8u*020u);machine.AllowInterrupt=n%2u;
  for(cycles=0;cycles<15000;++cycles){pc=get(RegZ);if(!machine.PendFlag && !machine.ExtraDelay && (pc==02070 || pc==02071) && get(RegFB)==014000)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==15000){fprintf(stderr,"relay%u timeout PC%05o CNT%05o BB%05o\n",n,pc,get(0776),get(RegBB));return 2;}
  printf("%u 0",n);
  for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2 ? 0177777u : 077777u));
  printf(" %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,wrote,channel10);
  fprintf(stderr,"relay%u %ucycles PC%05o BB%05o\n",n,cycles,pc,get(RegBB));
 }
 return 0;
}
