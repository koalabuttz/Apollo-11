/* SPDX-License-Identifier: GPL-2.0-or-later; host-only original source oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_abort_recovery_cases.h"
#include "cm_abort_recovery_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles,phase;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ABORT_RECOVERY_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned count=n&1u?1u:7u;static const unsigned targets[7]={05560,05740,05742,05744,02501,04675,07767};machine=baseline;cm_abort_recovery_input(&s,&io,n);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.InputChannel[014]=io.channel14;machine.InputChannel[015]=io.main_keys;machine.InputChannel[016]=io.navigation_keys;machine.Trap31A=io.trap31a;machine.Trap31B=io.trap31b;machine.Trap32=io.trap32;
  for(phase=0;phase<count;++phase){unsigned target=n&1u?07766u:targets[phase];
   for(cycles=0;cycles<1000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target&&(phase!=4u||get(RegFB)==012000u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==1000u){fprintf(stderr,"CM abort recovery timeout %lu phase%u PC%05o\n",(unsigned long)n,phase,get(RegZ));return 2;}
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_abort_recovery_dump(n,phase,EXEC_OK,&s,&io);
  }
 }return 0;
}
