/* SPDX-License-Identifier: MIT */
/* Native lm-ags-init cases in the oracle's format: case, kind, the 2048
 * erasable words and the interrupt-enable state. A refused case prints
 * kind AGS_REFUSED and its restored input image;
 * tools/lm_ags_init_freeze.py leaves those rows out of the comparison. */
#include <stdio.h>
#include "lm_ags_init_cases.h"
int main(void){unsigned n,i,kind;uint16_t ie;
 for(n=0;n<AGS_CASES;++n){apollo_overlay_state s;
  kind=ags_run(&s,(uint16_t)n,&ie);
  if(kind==3u){fprintf(stderr,"lm-ags-init case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  printf(" %u\n",(unsigned)ie);
 }return 0;}
