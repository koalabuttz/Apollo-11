/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "dsky_source_alarm_cases.h"
static agc_t ropes[2];static unsigned event;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)event=1;}
int main(int argc,char **argv){unsigned n,i,cycles,pc,route,target;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_SOURCE_ALARM_CASES;++n){apollo_overlay_state s;dsky_charin_io io;dsky_source_alarm_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];event=0;for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;
 route=n%8u>=5u ? 0u : n%8u==4u ? 4u : n%8u!=3u&&s.words[0136]==(n<4096u ? 04202u : 04216u) ? 3u : n%8u!=3u&&s.words[0136]==(n<4096u ? 04124u : 04140u) ? 2u : 1u;
 target=route==0u ? s.words[2] : route==1u ? (n<4096u ? 05155u : 05217u) : route==2u ? (n<4096u ? 04124u : 04140u) : route==3u ? (n<4096u ? 05652u : 05721u) : (n<4096u ? 03050u : 03272u);
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==target&&(route!=4u||get(RegFB)==020000u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==3000){fprintf(stderr,"SOURCEALARM%u timeout%05o\n",n,pc);return 2;}
 printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],event,route);
 }return 0;}
