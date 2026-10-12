/* SPDX-License-Identifier: MIT */
/* Native cm-stateup cases in the oracle's format (tools/cm_stateup_reference.c). */
#include <stdio.h>
#include "cm_stateup_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<CM_STATEUP_CASES;++n){apollo_overlay_state s;p66_bias_io io;uint16_t steps=0;
  if(cm_stateup_run(&s,&io,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"CM STATEUP %u stopped at %05o LOC %05o\n",n,(unsigned)s.words[5],(unsigned)s.words[0164]);return 1;}
  printf("%u %u %u",n,(unsigned)io.interrupt_enabled,(unsigned)io.superbank);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
