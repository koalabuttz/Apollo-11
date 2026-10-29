/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_unused_main
#include "reference_runner.c"
#undef main
#include "lm_radar_leadin_cases.h"
int main(int argc,char **argv){agc_t baseline;uint16_t n;
 if(argc!=2)return 2;
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<LM_RADAR_CASES;++n){apollo_overlay_state m;p66_bias_io io;unsigned i,cycles;enum lm_radar_result status=LM_RADAR_REJECT;
  lm_radar_input(&m,&io,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.interrupt_enabled;machine.OutputChannel7=machine.InputChannel[7]=io.superbank;
  for(cycles=0;cycles<2500u;++cycles){unsigned pc=get(RegZ);
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay){
    if(pc==05261){status=LM_RADAR_TASKOVER;break;}
    if(pc==04622){status=LM_RADAR_MEASURE;break;}
    if(pc==04616){status=LM_RADAR_STALL;break;}
    if(pc==05155){status=LM_RADAR_END;break;}
    if(pc==05716||pc==05726){status=LM_RADAR_ALARM;break;}
    if(pc==05203&&agc_correct((uint16_t)*memory(0))>WL_MAX_DELAY){status=LM_RADAR_WAITLIST;break;}
    if(pc==02003&&m.words[5]==02003&&m.words[02355]==AGC_MASK){status=LM_RADAR_REENTER;break;}
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
  }
  if(cycles==2500u){fprintf(stderr,"case%u timeoutpc%o\n",n,get(RegZ));return 2;}
  printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));printf(" %u %u\n",(unsigned)machine.OutputChannel7,machine.AllowInterrupt);
 }
 return 0;
}
