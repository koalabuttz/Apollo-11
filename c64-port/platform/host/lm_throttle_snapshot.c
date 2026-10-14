/* SPDX-License-Identifier: MIT */
/* Native lm-throttle cases in the oracle's format: case, outcome, channel 14
 * and all 2048 erasable words at the caller's return. */
#include <stdio.h>
#include "lm_throttle_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_THR_CASES;++n){apollo_overlay_state s;lm_flight_io io;uint16_t steps;
  if(lm_thr_run(&s,&io,(uint16_t)n,&steps)){fprintf(stderr,"lm-throttle case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u 0 %u",n,(unsigned)io.hw.channel14);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
  putchar('\n');
 }return 0;}
