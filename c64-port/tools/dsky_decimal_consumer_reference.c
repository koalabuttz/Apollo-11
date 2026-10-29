/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_decimal_consumer_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){static const agc_word cadr[2][14]={{061416,062566,060615,060677,060710,060715,060635,060717,065230,065303,060715,060704,060732,060623},{061323,062566,060516,060603,060614,060621,060524,060623,065143,065216,060621,060610,060636,060653}};unsigned n,i,cycles,pc,target,bank,routine,phase,route;uint32_t id;enum dsky_nv_mission mission;p66_bias_io io;agc_word channel;
 if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;
 InhibitAlarms=1;
 for(n=0;n<DSKY_DECIMAL_CONSUMER_CASES;++n){dsky_decimal_consumer_input(&s,&io,(uint16_t)n,&mission,&channel);routine=s.words[0140]==1u?((s.words[0147]>>5)&037u):(s.words[0153]&037u);machine=mission==DSKY_NV_LM?lm:cm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.InputChannel[033]=channel;machine.AllowInterrupt=io.interrupt_enabled;phase=0;id=n*32u;
  for(;;){route=0;bank=062000;target=02537;
   if(phase==1u){bank=cadr[mission][routine]&076000u;target=02000u+(cadr[mission][routine]&01777u);}
   else if(phase==2u)target=02566;
   else if(phase==3u){bank=060000;target=mission==DSKY_NV_LM?03226u:03133u;}
   else if(phase==4u){bank=060000;target=mission==DSKY_NV_LM?04631u:04675u;}
   else if(phase==5u)target=get(0117)?02537u:0136u;
   for(cycles=0;cycles<1500u;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay){if(get(4)==bank&&pc==target)break;if(phase==4u&&pc==(mission==DSKY_NV_LM?05705u:05747u)&&get(2)==(mission==DSKY_NV_LM?03354u:03261u)){route=2;break;}}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==1500u){fprintf(stderr,"consumer timeout n%u phase%u PC%oFB%oQ%o target%o\n",n,phase,pc,get(4),get(2),target);return 2;}
   if(phase==5u&&target==0136u)route=1;
   printf("%lu",(unsigned long)id++);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
   if(route)break;
   if(phase==5u)phase=1u;else ++phase;if(phase==2u&&routine==1u)phase=3u;
  }
 }return 0;}
