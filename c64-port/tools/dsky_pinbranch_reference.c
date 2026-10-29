/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_pinbranch_cases.h"
static agc_t ropes[2];
int main(int argc,char **argv){unsigned n,i,cycles,pc,route;static const unsigned targets[2][4]={{03471,02377,02544,02615},{03713,02624,02766,03037}};if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_PINBRANCH_CASES;++n){apollo_overlay_state s;p66_bias_io io;dsky_pinbranch_input(&s,&io,(uint16_t)n);machine=ropes[n/8192u];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);for(route=0;route<4;++route)if(((n/4096u)%2u ? pc==((n%4u)==0 ? (n<8192u ? 04154u : 04170u) : (n<8192u ? 03070u : 03312u))&&route==n%4u : pc==targets[n/8192u][route])&&get(RegFB)==020000u)break;if(route<4&&!machine.PendFlag&&!machine.ExtraDelay)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==3000){fprintf(stderr,"PINBRANCH%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
 }return 0;}
