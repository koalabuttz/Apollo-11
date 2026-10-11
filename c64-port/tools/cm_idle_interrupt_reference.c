/* SPDX-License-Identifier: GPL-2.0-or-later; original CM automatic interrupt/RESUME. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_idle_interrupt_cases.h"
#include "cm_idle_interrupt_dump.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_IDLE_INTERRUPT_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_idle_interrupt_state irq;machine=baseline;cm_idle_interrupt_input(&s,&io,&irq,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=irq.in_isr;if(n<2048u)machine.InterruptRequests[3]=1;
  for(cycles=0;cycles<1000;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&((n<2048u&&get(RegZ)==03423&&get(RegBB)==02003)||(n>=2048u&&machine.SubstituteInstruction)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==1000){fprintf(stderr,"CM IRQ timeout %u PC%05o BB%05o\n",n,get(RegZ),get(RegBB));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;irq.in_isr=(uint8_t)machine.InIsr;irq.pending_idle=(uint8_t)machine.SubstituteInstruction;cm_idle_interrupt_dump(n,n<2048u?0u:1u,&s,&io,&irq);
 }return 0;
}
