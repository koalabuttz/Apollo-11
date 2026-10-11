/* SPDX-License-Identifier: GPL-2.0-or-later; independent unchanged source execution. */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_display_cases.h"
static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<CM_DISPLAY_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned target,stage=n/4096u;
  cm_display_input(&s,&io,(uint16_t)n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  target=02000u+((s.words[0374]-3u)&01777u);
  for(cycles=0;cycles<2000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&((stage==0u&&pc==03300u)||(stage!=0u&&pc==target)||(stage!=2u&&pc>=02001u&&pc<=02006u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==2000){fprintf(stderr,"CM display case%u timeout%05o FB%05o\n",n,pc,get(RegFB));return 2;}
  printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
