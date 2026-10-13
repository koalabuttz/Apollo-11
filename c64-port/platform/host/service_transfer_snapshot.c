/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "service_transfer_cases.h"
int main(void){static apollo_overlay_state s;uint16_t n,i,superbank,inhint;
 for(n=0;n<svt_cases();++n){if(!svt_run(&s,&superbank,&inhint,n)){printf("%u 0\n",(unsigned)n);continue;}
  printf("%u 1 %u",(unsigned)n,(unsigned)inhint);for(i=0;i<2048u;++i)printf(" %u",(unsigned)(s.words[i]&(i<3u?0177777u:077777u)));printf("\n");}
 return 0;}
