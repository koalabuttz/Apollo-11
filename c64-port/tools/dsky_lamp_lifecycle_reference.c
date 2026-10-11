/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "dsky_lamp_lifecycle_cases.h"
static agc_t ropes[2];static unsigned event;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)event|=1u;if(c==013)event|=2u;}
int main(int argc,char **argv){unsigned n,i,cycles,pc,route,fb,cm,entry,alarm,waitalarm;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_LAMP_LIFECYCLE_CASES;++n){apollo_overlay_state s;dsky_lamp_io io;dsky_lamp_lifecycle_input(&s,&io,(uint16_t)n);cm=n>=4096u;entry=s.words[5];machine=ropes[cm];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.RestartLight=io.restart_light;machine.CycleCounter=1;event=0;
 for(cycles=0;cycles<4000;++cycles){pc=get(RegZ);fb=get(RegFB);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&((pc==(cm?05217u:05155u))||(pc==(cm?05324u:05261u))||(pc==(cm?07755u:05716u))||(fb==060000u&&pc==(cm?03412u:03505u))||(fb==020000u&&pc==(cm?03272u:03050u))))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 route=pc==(cm?05217u:05155u)?0u:pc==(cm?05324u:05261u)?1u:pc==(cm?07755u:05716u)?(entry==03662u||entry==03667u?4u:5u):fb==020000u?2u:3u;alarm=cm&&route==4u?((unsigned)machine.Fixed[1][get(RegQ)&01777u]&077777u):0u;waitalarm=0;
 if(cycles==4000){fprintf(stderr,"LAMP%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u %u %u %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],(unsigned)machine.InputChannel[012],(unsigned)machine.InputChannel[013],event,(unsigned)machine.RestartLight,alarm,waitalarm,route);
 }return 0;}
