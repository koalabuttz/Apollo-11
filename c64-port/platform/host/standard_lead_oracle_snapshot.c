/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "standard_lead_oracle_cases.h"
int main(void){static apollo_overlay_state s;uint16_t n,i,superbank,inhint;
 for(n=0;n<SLO_CASES;++n){if(!slo_run(&s,&superbank,&inhint,n)){printf("%u 0\n",(unsigned)n);continue;}
  printf("%u 1 %u %u",(unsigned)n,(unsigned)inhint,(unsigned)superbank);for(i=0;i<2048u;++i)printf(" %u",(unsigned)(s.words[i]&(i<3u?0177777u:077777u)));printf("\n");}
 return 0;}
