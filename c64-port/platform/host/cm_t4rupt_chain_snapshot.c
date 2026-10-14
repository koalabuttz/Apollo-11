/* SPDX-License-Identifier: MIT */
/* Native cm-t4rupt-chain cases in the oracle's format: case, outcome kind
 * and, unless refused, all 2048 erasable words, SUPERBNK, the interrupt
 * enable bit, InIsr and channels 10-14 and 30-33. */
#include <stdio.h>
#include "cm_t4rupt_chain_cases.h"
static const unsigned channels[9]={010,011,012,013,014,030,031,032,033};
int main(void){unsigned n,i,kind;uint16_t steps;
 for(n=0;n<CCH_CASES;++n){static apollo_overlay_state s;static cch_io io;
  kind=cch_run(&s,&io,(uint16_t)n,&steps);
  if(kind>CCH_REFUSED){fprintf(stderr,"cm-t4rupt-chain case %u: kind %u at %05o\n",n,kind,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=CCH_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u",(unsigned)io.cm.io.superbank,(unsigned)io.cm.io.interrupt_enabled,(unsigned)io.hw.in_interrupt);
   for(i=0;i<9u;++i)printf(" %u",(unsigned)cch_channel(&io,channels[i]));}
  putchar('\n');
 }return 0;}
