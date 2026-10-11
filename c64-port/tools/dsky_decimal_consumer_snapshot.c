/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_decimal_consumer_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;enum dsky_decimal_consumer_route route;agc_word channel,routine;uint16_t n,i;unsigned phase;uint32_t id;
 for(n=0;n<DSKY_DECIMAL_CONSUMER_CASES;++n){dsky_decimal_consumer_input(&s,&io,n,&mission,&channel);routine=(agc_word)(s.words[0140]==1u?((s.words[0147]>>5)&037u):(s.words[0153]&037u));phase=0;id=(uint32_t)n*32u;
  for(;;){enum dsky_decimal_consumer_stage stage=phase==0u?DSKY_DECIMAL_LOAD:phase==1u?DSKY_DECIMAL_SELECT:phase==2u?((routine==4u||routine==5u||routine==7u||routine==10u)?DSKY_DECIMAL_DP:DSKY_DECIMAL_SCALAR):phase==3u?DSKY_DECIMAL_CALL:phase==4u?DSKY_DECIMAL_FORMAT:DSKY_DECIMAL_RETURN;
   if(dsky_decimal_consumer(&s,&io,mission,stage,channel,&route)!=P66_BIAS_OK){fprintf(stderr,"unsupported n%u phase%u PC%o MPAC%o/%o routine%u\n",n,phase,s.words[5],s.words[0154],s.words[0155],routine);return 2;}
   printf("%lu",(unsigned long)id++);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);
   if(route!=DSKY_DECIMAL_NEXT)break;
   if(phase==5u)phase=1u;else ++phase;
   if(phase==2u&&routine==1u)phase=3u;
  }
 }return 0;}
