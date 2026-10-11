/* SPDX-License-Identifier: GPL-2.0-or-later; independent actual CM SETJTAG body. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_entry_tasks_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_ENTRYTASK_CASES;++n){apollo_overlay_state s;cm_native_io io;machine=baseline;cm_entrytask_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<1000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==05324)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==1000){fprintf(stderr,"CM SETJTAG %u timeout %05o\n",n,pc);return 2;}
  printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt);
 }return 0;
}
