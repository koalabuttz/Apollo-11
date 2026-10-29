/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_five_blank_cases.h"
static agc_t ropes[2];
int main(int argc,char **argv){unsigned n,i,cycles,pc,route;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_FIVE_BLANK_CASES;++n){apollo_overlay_state s;p66_bias_io io;dsky_five_blank_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((pc==(n<4096u ? 05705u : 05747u))||(pc==(n%4u==2 ? s.words[1] : s.words[2])&&get(RegQ)==2u&&get(RegFB)==060000u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 route=pc==(n<4096u ? 05705u : 05747u) ? 1u : 0u;
 if(cycles==3000){fprintf(stderr,"5BLANK%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
 }return 0;}
