/* SPDX-License-Identifier: MIT */
/* Native lm-exec-jobs cases in the oracle's format: case, outcome kind and,
 * unless refused, all 2048 erasable words, SUPERBNK, the interrupt-enable
 * bit, channel 11 and the interrupt-in-progress flag at the stop. */
#include <stdio.h>
#include "lm_exec_jobs_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_EXJ_CASES;++n){apollo_overlay_state s;p66_bias_io io;lm_exj_case c;uint16_t steps;
  kind=lm_exj_run(&s,&io,&c,(uint16_t)n,&steps);
  if(kind>LM_EXJ_REFUSED){fprintf(stderr,"lm-exec-jobs case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=LM_EXJ_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u %u",(unsigned)io.superbank,(unsigned)io.interrupt_enabled,(unsigned)c.hw.channel11,(unsigned)c.hw.in_interrupt);}
  putchar('\n');
 }return 0;}
