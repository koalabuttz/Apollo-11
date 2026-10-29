/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_keyboard_irq_cases.h"
static agc_t pristine;
int main(int argc,char **argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<CM_KEYBOARD_IRQ_CASES;++n){apollo_overlay_state s;cm_native_io io;uint16_t isr;cm_keyboard_irq_input(&s,&io,&isr,(uint16_t)n);machine=pristine;
  for(i=0;i<2048;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=isr;machine.InputChannel[015]=cm_keyboard_irq_channel15((uint16_t)n);
  for(cycles=0;cycles<3000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&(n<8192u ? pc==05333u : !machine.InIsr))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==3000){fprintf(stderr,"CM keyboard%u timeout%05o\n",n,pc);return 2;}
  printf("%u 0",n);for(i=0;i<2048;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InIsr);
 }return 0;}
