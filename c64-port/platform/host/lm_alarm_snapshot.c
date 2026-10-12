/* SPDX-License-Identifier: MIT */
/* Native lm-alarm cases in the oracle's format (tools/lm_alarm_reference.c). */
#include <stdio.h>
#include "lm_alarm_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_AL_CASES;++n){apollo_overlay_state s;p66_bias_io io;agc_hardware hw;uint16_t steps=0;
  if(lm_al_run(&s,&io,&hw,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"LM alarm %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u %u",n,(unsigned)io.interrupt_enabled,(unsigned)io.superbank,(unsigned)hw.in_interrupt);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
