/* SPDX-License-Identifier: MIT */
/* Native lm-v37 cases in the oracle's format (tools/lm_v37_reference.c). */
#include <stdio.h>
#include "lm_v37_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_V37_CASES;++n){apollo_overlay_state s;lm_fresh_io io;agc_hardware hw;uint16_t steps=0;
  if(lm_v37_run(&s,&io,&hw,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"LM V37 %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u %u %u %u %u %u %u %u %u %u",n,(unsigned)io.control.interrupt_enabled,(unsigned)io.control.superbank,(unsigned)hw.channel11,
         (unsigned)io.channel5,(unsigned)io.channel6,(unsigned)io.channel12,(unsigned)io.channel13,(unsigned)io.channel14,
         (unsigned)io.trap31a,(unsigned)io.trap31b,(unsigned)io.trap32);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
