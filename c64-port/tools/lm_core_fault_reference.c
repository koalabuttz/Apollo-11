/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "lm_core_fault_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint16_t n;if(argc!=2||agc_load_binfile(&baseline,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<LM_CORE_FAULT_CASES;++n){apollo_overlay_state image;p66_bias_io io;unsigned i,cycles,status=0,target;lm_core_fault_input(&image,&io,n);target=n%5u==4u?014u:7u;status=EXEC_UNMAPPED;machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<2000u;++cycles){unsigned pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(pc==target)break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 3;}if(cycles==2000u){fprintf(stderr,"case%u timeoutPC%o\n",n,get(RegZ));return 4;}
 printf("%u %u",n,status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
