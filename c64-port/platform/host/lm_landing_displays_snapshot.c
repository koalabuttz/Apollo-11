/* SPDX-License-Identifier: MIT */
/* Native lm-landing-displays cases in the oracle's format: case, outcome
 * kind, channels 12 and 14 and all 2048 erasable words at the stop. A
 * refused case prints kind LM_LAD_REFUSED and the unchanged input image;
 * tools/lm_landing_displays_freeze.py leaves those rows out of the oracle
 * comparison. */
#include <stdio.h>
#include "lm_landing_displays_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_LAD_CASES;++n){apollo_overlay_state s;lm_flight_io io;uint16_t steps;
  kind=lm_lad_run(&s,&io,(uint16_t)n,&steps);
  if(kind>=LM_LAD_NONE){fprintf(stderr,"lm-landing-displays case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u %u",n,kind,(unsigned)io.hw.channel12,(unsigned)io.hw.channel14);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
