/* SPDX-License-Identifier: GPL-2.0-or-later; uninterrupted original source. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_idle_wake_cases.h"
#include "cm_idle_interrupt_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,p,c;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_IDLE_WAKE_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_idle_interrupt_state irq;machine=baseline;cm_idle_wake_input(&s,&io,&irq,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[011]=io.channel11;machine.InIsr=0;
  for(p=0;p<9u;++p){static const unsigned target[9]={03231,03423,03761,05201,03765,05324,05341,03231,03332};if(p==1u)machine.InterruptRequests[3]=1;
   for(c=0;c<2000u;++c){if(c&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target[p]&&(p!=7u||machine.SubstituteInstruction)&&(p!=8u||get(RegBB)==050006u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(c==2000u){fprintf(stderr,"idle wake source timeout case%lu phase%u PC%05o BB%05o\n",(unsigned long)n,p,get(RegZ),get(RegBB));return 2;}
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.channel11=(agc_word)machine.InputChannel[011];io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;irq.in_isr=(uint8_t)machine.InIsr;irq.pending_idle=(uint8_t)machine.SubstituteInstruction;cm_idle_interrupt_dump(n*9u+p,0,&s,&io,&irq);
  }
 }return 0;
}
