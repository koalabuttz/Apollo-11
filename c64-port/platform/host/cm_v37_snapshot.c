/* SPDX-License-Identifier: MIT */
/* Native cm-v37 cases in the oracle's format (tools/cm_v37_reference.c). */
#include <stdio.h>
#include "cm_v37_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<CM_V37_CASES;++n){apollo_overlay_state s;cm_fresh_io io;uint16_t steps=0;
  if(cm_v37_run(&s,&io,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"CM V37 %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u %u %u %u %u %u %u %u %u %u %u",n,(unsigned)io.control.interrupt_enabled,(unsigned)io.control.superbank,(unsigned)io.control.channel11,
         (unsigned)io.channel5,(unsigned)io.channel6,(unsigned)io.control.channel12,(unsigned)io.control.channel13,(unsigned)io.control.channel14,
         (unsigned)io.channel33,(unsigned)io.control.trap31a,(unsigned)io.control.trap31b,(unsigned)io.control.trap32);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
