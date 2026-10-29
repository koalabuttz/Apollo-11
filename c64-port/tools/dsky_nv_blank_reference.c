/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_nv_blank_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){unsigned n,i,cycles,pc;enum dsky_nv_mission mission;agc_word channel11;p66_bias_io io;if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;InhibitAlarms=1;for(n=0;n<DSKY_NV_BLANK_CASES;++n){dsky_nv_blank_input(&s,&io,&channel11,(uint16_t)n,&mission);machine=mission==DSKY_NV_LM?lm:cm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.InputChannel[011]=channel11;machine.AllowInterrupt=io.interrupt_enabled;for(cycles=0;cycles<300u;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&pc==03566u)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==300u){fprintf(stderr,"NVblank timeout %u PC%oFB%o\n",n,pc,get(4));return 2;}printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011]);}return 0;}
