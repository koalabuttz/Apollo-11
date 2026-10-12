/* SPDX-License-Identifier: MIT */
/* Native lm-update cases in the oracle's format (tools/lm_update_reference.c). */
#include <stdio.h>
#include "lm_update_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_UP_CASES;++n){apollo_overlay_state s;p66_bias_io io;agc_hardware hw;uint16_t steps=0;
  if(lm_up_run(&s,&io,&hw,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"LM update %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u %u",n,(unsigned)io.interrupt_enabled,(unsigned)io.superbank,(unsigned)hw.channel11);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
