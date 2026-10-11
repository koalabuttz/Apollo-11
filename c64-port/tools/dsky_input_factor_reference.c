/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "dsky_input_factor_cases.h"
static agc_t ropes[2];static unsigned event;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)event|=1u;if(c==013)event|=2u;}
int main(int argc,char **argv){static const unsigned targets[2][14]={{02350,03106,03010,03076,03114,03141,03420,03114,03452,03420,03146,03111,03420,03010},{02350,02776,02716,02766,03021,03046,03004,03021,03365,03325,03053,03001,03325,03032}},banks[]={062000,060000,060000,060000,060000,060000,060000,060000,064000,060000,060000,060000,060000,060000};unsigned n,i,cycles,pc,route,cm,callerpc;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_INPUT_FACTOR_CASES;++n){apollo_overlay_state s;dsky_lamp_io io;dsky_input_factor_input(&s,&io,(uint16_t)n);cm=n>=4096u;machine=ropes[cm];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.RestartLight=io.restart_light;machine.CycleCounter=1;event=0;callerpc=s.words[5]==03207u?s.words[0]:s.words[5]==03174u&&s.words[0140]==2u?(s.words[0153]>>(s.words[0117]*5u))&037u:(s.words[0147]>>5)&037u;
 for(cycles=0;cycles<4000;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&pc==targets[cm][callerpc]&&get(RegFB)==banks[callerpc])break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 route=callerpc;
 if(cycles==4000){fprintf(stderr,"INPUT_ARITHMETIC%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],(unsigned)machine.InputChannel[012],(unsigned)machine.InputChannel[013],event,(unsigned)machine.RestartLight,route);
 }return 0;}
