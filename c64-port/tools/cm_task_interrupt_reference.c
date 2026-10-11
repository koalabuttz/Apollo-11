/* SPDX-License-Identifier: GPL-2.0-or-later; original CM automatic interrupt/RESUME. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_task_interrupt_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_TASK_INTERRUPT_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_task_interrupt_state irq;machine=baseline;cm_task_interrupt_input(&s,&io,&irq,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=irq.in_isr;if(n<512u)machine.InterruptRequests[3]=1;
  for(cycles=0;cycles<1000;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&((n<512u&&get(RegZ)==03423&&get(RegBB)==02003)||(n>=512u&&machine.SubstituteInstruction)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==1000){fprintf(stderr,"CM IRQ timeout %u PC%05o BB%05o\n",n,get(RegZ),get(RegBB));return 2;}
  printf("%u %u",n,n<512u?0u:1u);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));printf(" %u %u %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt,machine.InIsr,machine.SubstituteInstruction);
 }return 0;
}
