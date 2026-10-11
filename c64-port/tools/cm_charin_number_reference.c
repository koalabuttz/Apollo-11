/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "cm_charin_number_cases.h"
static agc_t pristine;static unsigned wrote,channel11;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011){wrote=1;channel11=(unsigned)v&077777u;}}
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<CM_CHARIN_NUMBER_CASES;++n){apollo_overlay_state s;cm_native_io io;uint16_t event;cm_charin_number_input(&s,&io,&event,(uint16_t)n);machine=pristine;wrote=0;channel11=io.channel11;machine.InputChannel[011]=(int)channel11;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==05217)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==3000){fprintf(stderr,"CM numeric%u timeout%05o\n",n,pc);return 2;}
  printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,channel11,wrote);
 }return 0;}
