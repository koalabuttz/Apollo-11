/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "dsky_lamp_test_cases.h"
static agc_t ropes[2];static unsigned event;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)event|=1u;if(c==013)event|=2u;}
int main(int argc,char **argv){unsigned n,i,cycles,pc,route,fb,cm,entry,caller;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_LAMP_TEST_CASES;++n){apollo_overlay_state s;dsky_lamp_io io;dsky_lamp_test_input(&s,&io,(uint16_t)n);cm=n>=4096u;entry=s.words[5];caller=s.words[2];machine=ropes[cm];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.RestartLight=io.restart_light;machine.CycleCounter=1;event=0;
 for(cycles=0;cycles<4000;++cycles){pc=get(RegZ);fb=get(RegFB);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&((pc==(cm?05245u:05203u))||(pc==(cm?05134u:05072u))||(fb==010000u&&pc==(cm?02566u:02636u))||(fb==060000u&&pc==(cm?03376u:03471u))||(fb==020000u&&pc==03272u)||(entry==02730u&&fb==066000u&&pc==caller)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 route=pc==(cm?05245u:05203u)?0u:pc==(cm?05134u:05072u)?1u:fb==010000u?2u:fb==060000u?3u:fb==020000u?4u:5u;
 if(cycles==4000){fprintf(stderr,"LAMP%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],(unsigned)machine.InputChannel[012],(unsigned)machine.InputChannel[013],event,(unsigned)machine.RestartLight,route);
 }return 0;}
