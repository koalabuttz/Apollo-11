/* SPDX-License-Identifier: MIT */
/* Native lm-stateup cases in the oracle's format (tools/lm_stateup_reference.c). */
#include <stdio.h>
#include "lm_stateup_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_STATEUP_CASES;++n){apollo_overlay_state s;p66_bias_io io;uint16_t steps=0;
  if(lm_stateup_run(&s,&io,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"LM STATEUP %u stopped at %05o LOC %05o\n",n,(unsigned)s.words[5],(unsigned)s.words[0164]);return 1;}
  printf("%u %u %u",n,(unsigned)io.interrupt_enabled,(unsigned)io.superbank);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
