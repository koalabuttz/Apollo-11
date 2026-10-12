/* SPDX-License-Identifier: MIT */
/* Native lm-burn-baby cases in the oracle's format: case, kind, the 2048
 * erasable words, channels 11, 14 and 30 and the interrupt-enable state.
 * A refused case prints kind LMBB_REFUSED and its restored input image;
 * tools/lm_burn_baby_freeze.py leaves those rows out of the comparison. */
#include <stdio.h>
#include "lm_burn_baby_cases.h"
int main(void){unsigned n,i,kind;lmbb_case c;
 for(n=0;n<LMBB_CASES;++n){apollo_overlay_state s;
  kind=lmbb_run(&s,(uint16_t)n,&c);
  if(kind==3u){fprintf(stderr,"lm-burn-baby case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u %u %u %u\n",(unsigned)c.chan11,(unsigned)c.chan14,(unsigned)c.chan30,(unsigned)c.interrupt_enabled);
 }return 0;}
