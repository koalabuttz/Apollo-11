/* SPDX-License-Identifier: MIT */
/* Native lm-tjet-law cases in the oracle's format: case, outcome and all
 * 2048 erasable words at the caller's return. */
#include <stdio.h>
#include "lm_tjet_law_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_TJ_CASES;++n){apollo_overlay_state s;uint16_t steps;
  if(lm_tj_run(&s,(uint16_t)n,&steps)){fprintf(stderr,"lm-tjet-law case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u 0",n);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
  putchar('\n');
 }return 0;}
