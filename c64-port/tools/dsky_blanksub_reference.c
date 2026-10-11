/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_blanksub_cases.h"
static agc_t ropes[2];
int main(int argc,char **argv){unsigned n,i,cycles,pc,target;if(argc!=3||agc_load_binfile(&ropes[0],argv[1])||agc_load_binfile(&ropes[1],argv[2]))return 2;InhibitAlarms=1;
 for(n=0;n<DSKY_BLANKSUB_CASES;++n){apollo_overlay_state s;p66_bias_io io;dsky_blanksub_input(&s,&io,(uint16_t)n);machine=ropes[n/4096u];for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;target=s.words[2]+(((s.words[01021]&020000u)+s.words[01012]) ? 0u : 1u);
 for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&pc==target&&get(RegFB)==s.words[4]&&get(RegQ)==(((s.words[01021]&020000u)+s.words[01012])||(s.words[0]&7u)==0u ? 3u : 2u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==3000){fprintf(stderr,"BLANK%u timeout%05o\n",n,pc);return 2;}printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
