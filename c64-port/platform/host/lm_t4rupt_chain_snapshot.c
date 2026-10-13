/* SPDX-License-Identifier: MIT */
/* Native lm-t4rupt-chain cases in the oracle's format: case, outcome kind
 * and, unless refused, all 2048 erasable words, SUPERBNK, the interrupt
 * enable bit, InIsr and channels 10, 11, 12, 14, 30 and 33. */
#include <stdio.h>
#include "lm_t4rupt_chain_cases.h"
int main(void){unsigned n,i,kind;uint16_t steps;
 for(n=0;n<LCH_CASES;++n){static apollo_overlay_state s;static lch_io io;
  kind=lch_run(&s,&io,(uint16_t)n,&steps);
  if(kind>LCH_REFUSED){fprintf(stderr,"lm-t4rupt-chain case %u ran past %u routines at %05o\n",n,LCH_MAX_STEPS,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=LCH_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u %u %u %u %u %u %u",(unsigned)io.lm.superbank,(unsigned)io.lm.interrupt_enabled,(unsigned)io.hw.in_interrupt,(unsigned)io.hw.channel10,
    (unsigned)io.hw.channel11,(unsigned)io.hw.channel12,(unsigned)io.hw.channel14,(unsigned)io.hw.channel30,(unsigned)io.hw.channel33);}
  putchar('\n');
 }return 0;}
