/* SPDX-License-Identifier: MIT */
/* Native lm-lunar-landing cases in the oracle's format: case, kind, the
 * 2048 erasable words, channels 12 and 33 and the interrupt-enable state.
 * A refused case prints kind LLD_REFUSED and its restored input image;
 * tools/lm_lunar_landing_freeze.py leaves those rows out of the comparison. */
#include <stdio.h>
#include "lm_lunar_landing_cases.h"
int main(void){unsigned n,i,kind;lld_case c;
 for(n=0;n<LLD_CASES;++n){apollo_overlay_state s;
  kind=lld_run(&s,(uint16_t)n,&c);
  if(kind==3u){fprintf(stderr,"lm-lunar-landing case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u %u %u\n",(unsigned)c.chan12,(unsigned)c.chan33,(unsigned)c.interrupt_enabled);
 }return 0;}
