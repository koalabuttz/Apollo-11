/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_pinball_cases.h"
static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<CM_PINBALL_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_pinball_input(&s,&io,(uint16_t)n);machine=pristine;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<2000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==s.words[2]&&get(RegQ)==(n%3u==2u?3u:0115u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000){fprintf(stderr,"CM pinball%u timeout%05o FB%05o\n",n,pc,get(RegFB));return 2;}
  printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
