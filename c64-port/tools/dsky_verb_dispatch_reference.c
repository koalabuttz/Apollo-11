/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_verb_dispatch_cases.h"
static agc_t ropes[2];
int main(int argc,char **argv){unsigned n,i,cycles,pc,route;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_VERB_DISPATCH_CASES;++n){apollo_overlay_state s;p66_bias_io io;dsky_verb_dispatch_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);route=(s.words[01001]!=077777u&&s.words[01001]>=40u)?1u:0u;if(!machine.PendFlag&&!machine.ExtraDelay&&(route ? pc==(n<4096u ? 04457u : 04473u)&&get(RegQ)==02143u : cycles&&pc>=02000u&&pc<=03777u&&get(RegQ)==pc-02000u&&get(RegA)==02151u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==3000){fprintf(stderr,"PINBRANCH%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
 }return 0;}
