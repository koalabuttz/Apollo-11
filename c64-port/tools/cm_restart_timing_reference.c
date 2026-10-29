/* SPDX-License-Identifier: GPL-2.0-or-later; independent original CM timing oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_restart_timing_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;
 if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;
 baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_RSTIME_CASES;++n){apollo_overlay_state s;unsigned status;
  machine=baseline;cm_rstime_input(&s,(uint16_t)n);
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=0100;machine.AllowInterrupt=0;
  for(cycles=0;cycles<200;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==0704)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==200){fprintf(stderr,"restart timing %u timeout %05o\n",n,pc);return 2;}
  status=get(0704)==05342?1u:0u;printf("%u %u",n,status);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
  fprintf(stderr,"restart timing %u %u cycles PC %05o\n",n,cycles,pc);
 }return 0;
}
