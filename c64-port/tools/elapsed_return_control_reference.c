/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_unused_main
#include "reference_runner.c"
#undef main
#include "elapsed_return_control_cases.h"
int main(int argc,char **argv){agc_t baseline;uint16_t n;
 if(argc!=2)return 2;
 memset(&machine,0,sizeof machine);
 if(agc_load_binfile(&machine,argv[1]))return 2;
 baseline=machine;InhibitAlarms=1;
 for(n=0;n<EQ_CASES;++n){apollo_overlay_state image;unsigned i,cycles;
  eq_prepare(&image,n);machine=baseline;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];
  machine.AllowInterrupt=(n&1u);machine.InputChannel[7]=(n&2u)?060:0;machine.OutputChannel7=machine.InputChannel[7];
  for(cycles=0;cycles<2500u;++cycles){
   if(!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==06061)break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
  }
  if(cycles==2500u)return 2;
  printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));
  printf(" %u %u\n",(unsigned)machine.InputChannel[7],machine.AllowInterrupt);
 }
 return 0;
}
