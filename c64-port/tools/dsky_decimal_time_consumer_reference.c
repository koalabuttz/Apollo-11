/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main reference_main
#include "cm_executive_reference.c"
#undef main
#include "dsky_decimal_time_consumer_cases.h"
static apollo_overlay_state s;static agc_t lm,cm;
int main(int argc,char **argv){static const agc_word targets[2][19]={{03234,03240,03242,03253,03255,03264,03266,0136,03343,03347,03351,03361,03371,02570,02733,02734,02736,02745,02570},{03147,03153,03155,03166,03170,03177,03201,0136,03256,03262,03264,03274,03304,02570,02637,02640,02642,02651,02570}};unsigned n,i,cycles,pc,target,bank,routine,stage,route;uint32_t id;enum dsky_nv_mission mission;p66_bias_io io;
 if(argc!=3||agc_load_binfile(&lm,argv[1])||agc_load_binfile(&cm,argv[2]))return 2;
 InhibitAlarms=1;
 for(n=0;n<DSKY_DECIMAL_TIME_CONSUMER_CASES;++n){dsky_decimal_time_consumer_input(&s,&io,(uint16_t)n,&mission);routine=s.words[0140]==1u?((s.words[0147]>>5)&037u):(s.words[0153]&037u);machine=mission==DSKY_NV_LM?lm:cm;for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;id=n*128u;
  for(;;){stage=dsky_decimal_time_consumer_stage(mission,get(RegZ));route=0;bank=get(4);
   if(stage==0u)target=02537;
   else if(stage==1u){if(routine==8u){target=mission==DSKY_NV_LM?03230u:03143u;bank=064000;}else if(routine==9u){target=mission==DSKY_NV_LM?03303u:03216u;bank=064000;}else{target=mission==DSKY_NV_LM?02732u:02636u;bank=060000;}}
   else if(stage==21u)target=get(0117)?02537u:0136u;
   else if(stage>=2u&&stage<=20u){target=targets[mission][stage-2u];if(stage==15u||stage==20u)bank=062000;}
   else return 2;
   for(cycles=0;cycles<1500u;++cycles){pc=get(RegZ);if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&get(4)==bank&&pc==target)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
   if(cycles==1500u){fprintf(stderr,"time consumer timeout n%u stage%u PC%oFB%oQ%o target%o\n",n,stage,pc,get(4),get(2),target);return 2;}
   if(target==0136u)route=1;
   printf("%lu",(unsigned long)id++);for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<=2u?0177777u:077777u));printf(" %u %u %u\n",(unsigned)machine.OutputChannel7,(unsigned)machine.AllowInterrupt,route);
   if(route)break;
  }
 }return 0;}
