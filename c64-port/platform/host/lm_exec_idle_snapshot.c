/* SPDX-License-Identifier: MIT */
/* Native lm-exec-idle cases in the oracle's format: case, outcome kind and,
 * unless refused, all 2048 erasable words, SUPERBNK, the interrupt-enable
 * bit, channel 11 and InIsr at the continuation. */
#include <stdio.h>
#include "lm_exec_idle_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_EXI_CASES;++n){apollo_overlay_state s;p66_bias_io io;agc_hardware hw;
  kind=lm_exi_run(&s,&io,&hw,(uint16_t)n);
  if(kind>LM_EXI_REFUSED){fprintf(stderr,"lm-exec-idle case %u not entered at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=LM_EXI_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u %u",(unsigned)io.superbank,(unsigned)io.interrupt_enabled,(unsigned)hw.channel11&077777u,(unsigned)hw.in_interrupt);}
  putchar('\n');
 }return 0;}
