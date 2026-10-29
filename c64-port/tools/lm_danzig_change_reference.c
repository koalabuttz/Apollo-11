/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "lm_danzig_change_cases.h"
static agc_t baseline;
int main(int argc,char**argv){uint16_t n;if(argc!=2||agc_load_binfile(&baseline,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<LM_DANZIG_CHANGE_CASES;++n){apollo_overlay_state image;p66_bias_io io;unsigned i,cycles,offset,loc,bank,interpretive,target;lm_danzig_change_input(&image,&io,n);offset=image.words[067];loc=image.words[0164+offset];bank=image.words[0165+offset];interpretive=(loc&AGC_SIGN)!=0;if(interpretive)loc=agc_add_word(agc_negate(loc),1);target=interpretive?06070u:loc;machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<2000u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 3;}if(cycles==2000u){fprintf(stderr,"case%u timeoutPC%o\n",n,get(RegZ));return 4;}
 printf("%u %u %u %u",n,loc,bank,interpretive);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
