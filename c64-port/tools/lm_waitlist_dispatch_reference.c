/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "lm_waitlist_dispatch_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint16_t n;if(argc!=2||agc_load_binfile(&baseline,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<LM_WAITLIST_DISPATCH_COUNT;++n){apollo_overlay_state image;p66_bias_io io;agc_word c11,c14;unsigned i,cycles,target;lm_waitlist_dispatch_input(&image,&io,&c11,&c14,n);target=image.words[01410];machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InIsr=1;machine.InputChannel[011]=c11;machine.InputChannel[014]=c14;
  for(cycles=0;cycles<300u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 3;}if(cycles==300u)return 4;
  printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],(unsigned)machine.InputChannel[014],(unsigned)machine.InIsr);
 }return 0;}
