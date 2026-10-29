/* SPDX-License-Identifier: MIT */
/* Native cm-exec-services cases in the oracle's format: case, outcome kind
 * and, unless refused, all 2048 erasable words, SUPERBNK, the
 * interrupt-enable bit and the interrupt-in-progress flag at the stop. */
#include <stdio.h>
#include "cm_exec_services_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<CM_EXS_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_exs_case c;uint16_t steps;
  kind=cm_exs_run(&s,&io,&c,(uint16_t)n,&steps);
  if(kind>CM_EXS_REFUSED){fprintf(stderr,"cm-exec-services case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=CM_EXS_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u",(unsigned)io.superbank,(unsigned)io.interrupt_enabled,(unsigned)c.hw.in_interrupt);}
  putchar('\n');
 }return 0;}
