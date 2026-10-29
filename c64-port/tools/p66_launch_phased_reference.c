/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "p66_launch_cases.h"
static agc_t pristine;static apollo_overlay_state state;
static void dump(unsigned n,unsigned stage,unsigned status){unsigned i;printf("%u %u %u",n,stage,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",(unsigned)*memory(i)&(i<=2?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);}
int main(int argc,char **argv){unsigned n,stage,i,cycles,pc,status;static const unsigned ends[3]={02505,02536,03275};if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<P66_LAUNCH_CASE_COUNT;++n){machine=pristine;p66_launch_prepare(&state,(uint16_t)n);for(i=0;i<APOLLO_SHARED_WORDS;++i)*memory(i)=(int16_t)state.words[i];machine.OutputChannel7=machine.InputChannel[7]=(int)((n/5u)%4u*020u);machine.AllowInterrupt=n%2u;
  for(stage=0;stage<3;++stage){status=0;for(cycles=0;cycles<15000u;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(pc==ends[stage]&&get(RegFB)==(stage==2?064000:062000))break;if(pc==05716){status=(unsigned)machine.Fixed[1][get(RegQ)&01777]&077777u;break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}if(cycles==15000u){fprintf(stderr,"launch%u stage%u PC%05o timeout\n",n,stage,pc);return 3;}dump(n,stage,status);if(status)break;}
 }return 0;}
