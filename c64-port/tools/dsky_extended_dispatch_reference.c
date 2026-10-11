/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_extended_dispatch_cases.h"
static agc_t ropes[2];
int main(int argc,char **argv){unsigned n,i,cycles,pc,fan,seen;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_EXTENDED_DISPATCH_CASES;++n){apollo_overlay_state s;p66_bias_io io;dsky_extended_dispatch_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 fan=02002u+(s.words[0154]==077777u ? 0u : s.words[0154]);seen=0;
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(seen&&(get(RegQ)!=02002u||pc!=fan))break;if(pc==fan&&get(RegFB)==066000u)seen=1;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==3000){fprintf(stderr,"EXTENDED%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
