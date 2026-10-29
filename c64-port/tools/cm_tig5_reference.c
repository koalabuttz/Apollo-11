/* SPDX-License-Identifier: GPL-2.0-or-later; original CM TIG-5 program. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_tig5_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_TIG5_CASES;++n){apollo_overlay_state s;cm_native_io io;unsigned status;machine=baseline;cm_tig5_input(&s,&io,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.InputChannel[7]=machine.OutputChannel7=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<1000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&(pc==05324||pc==05245||pc==05147))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==1000){fprintf(stderr,"CM TIG-5 %u timeout %05o\n",n,pc);return 2;}status=pc==05245?0u:pc==05147?1u:2u;
  printf("%u %u",n,status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt);
 }return 0;
}
