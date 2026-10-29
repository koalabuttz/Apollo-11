/* SPDX-License-Identifier: GPL-2.0-or-later; independent original CM long-call oracle. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_longcall_cases.h"
static agc_t baseline;
int main(int argc,char **argv){unsigned n,i,cycles,pc=0;if(argc!=2)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<CM_LONGCALL_CASES;++n){apollo_overlay_state s;exec_address task;unsigned target,expectedbb;
  machine=baseline;cm_longcall_input(&s,&task,(uint16_t)n);for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=0100;machine.AllowInterrupt=0;
  target=n<2048u?05245:s.words[n<3072u?01434:01133];expectedbb=n<2048u?02003:s.words[n<3072u?01435:01134]&076007u;
  for(cycles=0;cycles<200;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==target&&get(RegBB)==expectedbb)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==200){fprintf(stderr,"CM long-call %u timeout PC %05o BB %05o target %05o BB %05o\n",n,pc,get(RegBB),target,expectedbb);return 2;}
  if(machine.OutputChannel7!=0100||machine.AllowInterrupt)return 2;
  printf("%u %u",n,n<2048u?0u:1u);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i==0||i==1?0177777u:077777u));putchar('\n');
  fprintf(stderr,"CM long-call %u cycles %u PC %05o BB %05o\n",n,cycles,pc,get(RegBB));
 }return 0;
}
