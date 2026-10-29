/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "cm_relay_cases.h"
static agc_t pristine;static unsigned wrote,channel10;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==010){wrote=1;channel10=(unsigned)v&077777u;}}
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<CM_RELAY_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_relay_output out;cm_relay_input(&s,&io,&out,(uint16_t)n);machine=pristine;wrote=0;channel10=out.channel10;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<2000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&(pc==s.words[2]||pc==s.words[2]+1u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000){fprintf(stderr,"CM relay%u timeout%05o\n",n,pc);return 2;}
  printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,wrote,channel10);
 }return 0;}
