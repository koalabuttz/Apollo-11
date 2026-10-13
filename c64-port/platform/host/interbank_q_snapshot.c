/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "interbank_q_cases.h"
int main(void){static apollo_overlay_state s;uint16_t n,i,superbank;
 for(n=0;n<IBQ_CASES;++n){if(!ibq_run(&s,&superbank,n)){printf("%u 0\n",(unsigned)n);continue;}
  printf("%u 1",(unsigned)n);for(i=0;i<2048u;++i)printf(" %u",(unsigned)(s.words[i]&(i<3u?0177777u:077777u)));printf("\n");}
 return 0;}
