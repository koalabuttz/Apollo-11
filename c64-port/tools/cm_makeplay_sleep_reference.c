/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_makeplay_sleep_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<CM_MAKEPLAY_SLEEP_CASES;++n){apollo_overlay_state s;cm_native_io io;machine=baseline;cm_makeplay_sleep_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.InputChannel[011]=io.channel11;machine.AllowInterrupt=io.interrupt_enabled;for(cycles=0;cycles<1000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((n<2048u&&pc==05201u)||(n>=2048u&&n<4096u&&pc==05175u)||(n>=4096u&&pc==03312u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==1000u){fprintf(stderr,"CM makeplay case%u timeout%05o\n",n,pc);return 2;}printf("%u %u",n,pc==05201u?0u:pc==05175u?1u:2u);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt,(unsigned)machine.InputChannel[011]);}return 0;}
