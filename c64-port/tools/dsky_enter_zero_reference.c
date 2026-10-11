/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_enter_zero_cases.h"
static agc_t ropes[2];
int main(int argc,char **argv){unsigned n,i,cycles,pc,route;static const unsigned targets[5]={02133,02221,02351,02064,02120};if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_ENTER_ZERO_CASES;++n){apollo_overlay_state s;p66_bias_io io;dsky_enter_zero_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);for(route=0;route<5;++route)if(pc==targets[route]&&get(RegFB)==062000u)break;if(route<5&&!machine.PendFlag&&!machine.ExtraDelay)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==3000){fprintf(stderr,"PINBRANCH%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
 }return 0;}
