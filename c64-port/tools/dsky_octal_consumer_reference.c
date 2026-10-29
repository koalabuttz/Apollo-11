/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_octal_consumer_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){unsigned n,i,k,c,pc;enum dsky_nv_mission mission;p66_bias_io io;
 if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;
 InhibitAlarms=1;
 for(n=0;n<DSKY_OCTAL_CONSUMER_CASES;++n){dsky_octal_consumer_input(&s,&io,(uint16_t)n,&mission);machine=mission==DSKY_NV_LM?lm:cm;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;machine.InputChannel[s.words[01017]&0777u]=s.words[01020];
  for(k=0;k<10u;++k){for(c=0;c<1000u;++c){pc=get(RegZ);if(c&&!machine.PendFlag&&!machine.ExtraDelay&&((get(4)==062000u&&(pc==03363u||pc==02422u||pc==02504u||pc==0136u||pc==02351u))||(pc==(mission==DSKY_NV_LM?05705u:05747u)&&get(4)==060000u)))break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(c==1000u){fprintf(stderr,"octal consumer timeout %u/%u PC%o\n",n,k,pc);return 2;}
   printf("%u",n*16u+k);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,pc==0136u?1u:pc==02351u?2u:(pc==05705u||pc==05747u)?3u:0u);
   if(pc==0136u||pc==02351u||pc==05705u||pc==05747u)break;
  }
  if(k==10u)return 2;
 }
 return 0;
}
