/* SPDX-License-Identifier: GPL-2.0-or-later; actual original priority entry. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_priority_complete_cases.h"
#include "cm_fixed_compat_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_PRIORITY_COMPLETE_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned winner=(n/2u)%7u,selected_interpreter=(n&1u)&&winner,target,bank;machine=baseline;cm_priority_complete_input(&s,&io,n);target=selected_interpreter?06037u:winner?s.words[0164u+winner*12u]:s.words[2];bank=winner?s.words[0165u+winner*12u]:(s.words[6]|io.superbank);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<1000u;++cycles){unsigned z=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&z==target&&get(RegBB)==(bank&076007u)&&machine.OutputChannel7==(bank&0160u))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==1000u){fprintf(stderr,"CM priority timeout %lu PC%05o\n",(unsigned long)n,get(RegZ));return 2;}
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_fixed_dump(n,(uint16_t)(n&1u),EXEC_OK,&s,&io);
 }return 0;
}
