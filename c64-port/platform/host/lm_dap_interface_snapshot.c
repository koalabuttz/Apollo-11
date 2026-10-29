/* SPDX-License-Identifier: MIT */
/* Native lm-dap-interface cases in the oracle's format: case, outcome kind
 * and all 2048 erasable words at the stop. */
#include <stdio.h>
#include "lm_dap_interface_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_DAP_CASES;++n){apollo_overlay_state s;uint16_t steps;
  kind=lm_dap_run(&s,(uint16_t)n,&steps);
  if(kind>LM_DAP_ALARM){fprintf(stderr,"lm-dap-interface case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
