/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_super_dispatch_cases.h"
#include "cm_alarm2_image_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_SUPER_DISPATCH_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned loc,bank;machine=baseline;cm_super_dispatch_input(&s,&io,n);loc=n>=28672u?02653:agc_correct(s.words[n&1u?1:0]);bank=n>=28672u?066107:agc_correct(s.words[n&1u?0:1]);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=n%2u;
  for(cycles=0;cycles<100u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==loc&&get(RegQ)==2u&&get(RegBB)==(bank&076007u)&&machine.OutputChannel7==(bank&0160u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==100u){fprintf(stderr,"CM super dispatch timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}if(machine.InIsr!=(int)(n%2u))return 2;
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_alarm2_image_dump(n,EXEC_OK,&s,&io);
 }return 0;
}
