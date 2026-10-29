/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_decimal_time_consumer_cases.h"
static apollo_overlay_state s;
int main(void){p66_bias_io io;enum dsky_nv_mission mission;enum dsky_decimal_consumer_route route;uint16_t n,i;uint8_t stage;uint32_t id;
 for(n=0;n<DSKY_DECIMAL_TIME_CONSUMER_CASES;++n){dsky_decimal_time_consumer_input(&s,&io,n,&mission);id=(uint32_t)n*128u;
  for(;;){stage=dsky_decimal_time_consumer_stage(mission,s.words[5]);
   if(dsky_decimal_time_consumer(&s,&io,mission,stage,&route)!=P66_BIAS_OK){fprintf(stderr,"unsupported n%u stage%u PC%o MPAC%o/%o\n",n,stage,s.words[5],s.words[0154],s.words[0155]);return 2;}
   printf("%lu",(unsigned long)id++);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);
   if(route!=DSKY_DECIMAL_NEXT)break;
  }
 }return 0;}
