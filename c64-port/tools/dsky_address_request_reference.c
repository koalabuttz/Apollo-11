/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#define ChannelOutput original_channel_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
#include "dsky_address_request_cases.h"
static agc_t ropes[2];static unsigned event;
void ChannelOutput(agc_t *s,int c,int v){original_channel_output(s,c,v);if(c==011)event=1;}
int main(int argc,char **argv){unsigned n,i,cycles,pc,route;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_ADDRESS_REQUEST_CASES;++n){apollo_overlay_state s;dsky_charin_io io;dsky_address_request_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.control.superbank;machine.AllowInterrupt=io.control.interrupt_enabled;machine.InputChannel[011]=io.channel11;event=0;
 for(cycles=0;cycles<4000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((pc==02133u&&get(RegFB)==062000u)||pc==(n<4096u ? 05155u : 05217u)||pc==(n<4096u ? 04202u : 04216u)||pc==(n<4096u ? 04124u : 04140u)||pc==(n<4096u ? 04145u : 04161u)||pc==(n<4096u ? 05705u : 05747u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 route=pc==02133u ? 0u : pc==(n<4096u ? 05155u : 05217u) ? 1u : pc==(n<4096u ? 04202u : 04216u) ? 2u : pc==(n<4096u ? 04124u : 04140u) ? 3u : pc==(n<4096u ? 04145u : 04161u) ? 4u : 5u;
 if(cycles==4000){fprintf(stderr,"REQADD%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],event,route);
 }return 0;}
