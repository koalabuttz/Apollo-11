/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_octal_consumer_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,k;enum dsky_nv_mission mission;enum dsky_octal_consumer_stage stage;p66_bias_io io;enum dsky_octal_consumer_route route;
 for(n=0;n<DSKY_OCTAL_CONSUMER_CASES;++n){dsky_octal_consumer_input(&s,&io,n,&mission);stage=DSKY_OCTAL_BEGIN;
  for(k=0;k<10u;++k){if(dsky_octal_consumer(&s,&io,mission,stage,s.words[01020],&route)!=P66_BIAS_OK)return 2;
   printf("%u",n*16u+k);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,(unsigned)route);
   if(route!=DSKY_OCTAL_NEXT)break;
   stage=s.words[5]==02504u?DSKY_OCTAL_CHANNEL:s.words[5]==03363u?DSKY_OCTAL_FORMAT:DSKY_OCTAL_RETURN;
  }
  if(k==10u)return 2;
 }
 return 0;
}
