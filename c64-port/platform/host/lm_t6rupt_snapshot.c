/* SPDX-License-Identifier: MIT */
/* Native lm-t6rupt cases in the oracle's format: case, outcome kind and,
 * unless refused, all 2048 erasable words, SUPERBNK, the interrupt-enable
 * bit, InIsr and channels 5, 6 and 13 at the continuation. */
#include <stdio.h>
#include "lm_t6rupt_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_T6_CASES;++n){apollo_overlay_state s;p66_bias_io io;agc_hardware hw;
  kind=lm_t6_run(&s,&io,&hw,(uint16_t)n);
  if(kind>LM_T6_REFUSED){fprintf(stderr,"lm-t6rupt case %u not entered at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=LM_T6_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u %u %u %u",(unsigned)io.superbank,(unsigned)io.interrupt_enabled,(unsigned)hw.in_interrupt,(unsigned)hw.channel5,(unsigned)hw.channel6,(unsigned)hw.channel13);}
  putchar('\n');
 }return 0;}
