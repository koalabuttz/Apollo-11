/* SPDX-License-Identifier: MIT */
/* Native cm-exec-jobs cases in the oracle's format: case, outcome kind and,
 * unless refused, all 2048 erasable words, SUPERBNK, the interrupt-enable bit
 * and channel 11 at the continuation. */
#include <stdio.h>
#include "cm_exec_jobs_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<CM_EXJ_CASES;++n){apollo_overlay_state s;cm_native_io io;
  kind=cm_exj_run(&s,&io,(uint16_t)n);
  if(kind>CM_EXJ_REFUSED){fprintf(stderr,"cm-exec-jobs case %u not entered at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=CM_EXJ_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u",(unsigned)io.superbank,(unsigned)io.interrupt_enabled,(unsigned)io.channel11&077777u);}
  putchar('\n');
 }return 0;}
