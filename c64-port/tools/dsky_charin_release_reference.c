/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "dsky_charin_release_cases.h"
static agc_t ropes[2];static unsigned event;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)event=1;}
int main(int argc,char **argv){unsigned n,i,cycles,pc,route;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_CHARIN_RELEASE_CASES;++n){apollo_overlay_state s;dsky_charin_io io;dsky_charin_release_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];event=0;for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&(pc==(n<4096u ? 05155u : 05217u)||(pc==(n<4096u ? 03050u : 03272u)&&get(RegFB)==020000u)||(pc==(n<4096u ? 03505u : 03412u)&&get(RegFB)==060000u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==3000){fprintf(stderr,"RELEASE%u timeout%05o\n",n,pc);return 2;}
 route=pc==(n<4096u ? 05155u : 05217u)?0u:pc==(n<4096u ? 03050u : 03272u)?1u:2u;printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],event,route);
 }return 0;}
