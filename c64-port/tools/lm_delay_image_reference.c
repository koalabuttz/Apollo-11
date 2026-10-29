/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "lm_delay_image_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint16_t n;if(argc!=2||agc_load_binfile(&baseline,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<LM_DELAY_IMAGE_CASES;++n){apollo_overlay_state image;p66_bias_io io;unsigned i,cycles,status=0,target;lm_delay_image_input(&image,&io,n);target=n%3u==2u?((image.words[061]+2u)&07777u):05261u;machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<2000u;++cycles){unsigned pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay){if(pc==target)break;if(pc==05716u||pc==05726u){status=(unsigned)machine.Fixed[1][get(RegQ)&01777u]&AGC_MASK;break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 3;}if(cycles==2000u){fprintf(stderr,"case%u timeoutPC%o\n",n,get(RegZ));return 4;}
 printf("%u %u",n,status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt);
 }return 0;}
