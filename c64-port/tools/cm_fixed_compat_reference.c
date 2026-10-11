/* SPDX-License-Identifier: GPL-2.0-or-later; actual original caller/SUPDACAL proof. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_fixed_compat_cases.h"
#include "cm_fixed_compat_dump.h"
static agc_t baseline;
int main(int argc,char **argv){uint32_t n;unsigned i,phase,cycles;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_FIXED_COMPAT_CASES;++n){apollo_overlay_state s;cm_native_io io;machine=baseline;cm_fixed_compat_input(&s,&io,n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(phase=0;phase<2u;++phase){unsigned target=phase==0u?04715u:n<40960u?03361u:03542u;
   for(cycles=0;cycles<200u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==200u){fprintf(stderr,"original fixed caller timeout %lu/%u PC%05o BB%05o\n",(unsigned long)n,phase,get(RegZ),get(RegBB));return 2;}
   for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<=2u?0177777u:077777u));io.superbank=(agc_word)machine.OutputChannel7;io.interrupt_enabled=(agc_word)machine.AllowInterrupt;cm_fixed_dump(n,(uint16_t)phase,(unsigned)EXEC_OK,&s,&io);
  }
 }return 0;}
