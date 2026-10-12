/* SPDX-License-Identifier: MIT */
/* Native lm-dapidler cases in the oracle's format: case, outcome kind,
 * channels 5, 6, 12, 14 and all 2048 erasable words at the stop. */
#include <stdio.h>
#include "lm_dapidler_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_DI_CASES;++n){apollo_overlay_state s;lm_flight_io io;uint16_t steps;
  kind=lm_di_run(&s,&io,(uint16_t)n,&steps);
  if(kind>=LM_DI_NONE){fprintf(stderr,"lm-dapidler case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u %u %u %u",n,kind,(unsigned)io.chan5,(unsigned)io.chan6,(unsigned)io.chan12,(unsigned)io.chan14);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
