/* SPDX-License-Identifier: MIT */
/* Native lm-servicer cases in the oracle's format: case, kind, the 2048
 * erasable words, channels 11, 12, 32 and 33 and the interrupt-enable
 * state. A refused case prints kind LMSV_REFUSED and its restored input
 * image; tools/lm_servicer_freeze.py leaves those rows out of the
 * comparison. Built with LMSV_C64_GROUP (a C64 segment group), it prints
 * only the cases of that group's keys, each run to the end of its segment. */
#include <stdio.h>
#include "lm_servicer_cases.h"
int main(void){unsigned n,i,kind;lmsv_case c;
 for(n=0;n<LMSV_CASES;++n){apollo_overlay_state s;
  kind=lmsv_run(&s,(uint16_t)n,&c);
  if(kind==4u)continue;   /* a key of another group */
  if(kind==3u){fprintf(stderr,"lm-servicer case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u %u %u %u %u\n",(unsigned)c.chan11,(unsigned)c.chan12,(unsigned)c.chan32,(unsigned)c.chan33,(unsigned)c.interrupt_enabled);
 }return 0;}
