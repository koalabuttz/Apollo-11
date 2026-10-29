/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main idle_original_unused_main
#include "reference_runner.c"
#undef main
#include "lm_idle_dispatch_cases.h"
static apollo_overlay_state image;static agc_t pristine;
int main(int argc,char**argv){unsigned n,i,cycles,pc;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;
 InhibitAlarms=1;for(n=0;n<LM_IDLE_DISPATCH_CASES;++n){p66_bias_io io;agc_word channel;
  lm_idle_dispatch_input(&image,&io,&channel,(uint16_t)n);machine=pristine;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)image.words[i];
  machine.OutputChannel7=machine.InputChannel[7]=io.superbank;
  machine.InputChannel[011]=channel;machine.AllowInterrupt=io.interrupt_enabled;
  for(cycles=0;cycles<2000u;++cycles){pc=get(RegZ);
   if(!machine.PendFlag&&!machine.ExtraDelay&&(pc==03340u||pc==06070u||(pc>=02001u&&pc<=02007u)))break;
   machine.ScalerCounter=0;if(agc_engine(&machine))return 3;}
  if(cycles==2000u){fprintf(stderr,"case%u timeoutPC%05o\n",n,pc);return 4;}
  printf("%u %u %u %u %u",n,(unsigned)(pc==03340u?EXEC_IDLE:EXEC_OK),
   (unsigned)(pc==06070u),(unsigned)(pc==06070u?get(0164):pc),
   (unsigned)(get(RegBB)|machine.OutputChannel7));
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));
  printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,
   (unsigned)machine.InputChannel[011]);}return 0;}
