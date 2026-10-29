/* SPDX-License-Identifier: MIT */
/* Native lm-exec-services cases in the oracle's format: case, outcome kind
 * and, unless refused, all 2048 erasable words at the stop. */
#include <stdio.h>
#include "lm_exec_services_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_EXS_CASES;++n){apollo_overlay_state s;uint16_t steps;
  kind=lm_exs_run(&s,(uint16_t)n,&steps);
  if(kind>LM_EXS_REFUSED){fprintf(stderr,"lm-exec-services case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=LM_EXS_REFUSED)for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
