/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main inherited_reference_main
#include "reference_runner.c"
#undef main
#include "p66_application_job_cases.h"
static agc_t baseline;
int main(int argc,char **argv){uint16_t n;if(argc!=2||agc_load_binfile(&baseline,argv[1]))return 2;InhibitAlarms=1;
 for(n=0;n<P66_APPLICATION_JOB_COUNT;++n){apollo_overlay_state image;p66_bias_io io;agc_word c11,c14;unsigned i,cycles;
  p66_application_job_input(&image,&io,&c11,&c14,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];
  machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[011]=c11;machine.InputChannel[014]=c14;
  for(cycles=0;cycles<30000u;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==03214u&&get(RegFB)==02000u)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 3;}
  if(cycles==30000u){fprintf(stderr,"case%u source timeout PC%o FB%o\n",n,get(RegZ),get(RegFB));return 4;}
  printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=1u?0177777u:077777u));
  printf(" %u %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,(unsigned)machine.InputChannel[011],(unsigned)machine.InputChannel[014]);
 }return 0;
}
