/* SPDX-License-Identifier: GPL-2.0-or-later; original CM context changes. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_context_change_cases.h"
#include "cm_fixed_compat_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_CONTEXT_CHANGE_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned slot=(n/4u)%6u+1u,selected_interpreter=(n/2u)%2u,target,bank;machine=baseline;cm_context_change_input(&s,&io,n);target=selected_interpreter?06037u:s.words[0164u+slot*12u];bank=s.words[0165u+slot*12u];
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<1000u;++cycles){unsigned z=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&z==target&&get(RegBB)==(bank&076007u)&&machine.OutputChannel7==(bank&0160u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==1000u){fprintf(stderr,"CM context change timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_fixed_dump(n,(uint16_t)(n&1u),EXEC_OK,&s,&io);
 }return 0;
}
