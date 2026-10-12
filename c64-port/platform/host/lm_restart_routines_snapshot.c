/* SPDX-License-Identifier: MIT */
/* Native RESTARTS compositions in the oracle's format: case, service calls
 * (service A L Q 2CADR) and all 2048 erasable words at SWRETURN. */
#include <stdio.h>
#include "lm_restart_routines_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_RSR_CASES;++n){apollo_overlay_state s;lm_rsr_log log;uint16_t steps;
  if(lm_rsr_run(&s,(uint16_t)n,&log,&steps)!=EXEC_OK){fprintf(stderr,"LM restart composition %u stopped\n",n);return 1;}
  printf("%u %u",n,(unsigned)log.calls);
  for(i=0;i<log.calls;++i)printf(" %u %u %u %u %u %u",(unsigned)log.call[i].service,(unsigned)log.call[i].a,(unsigned)log.call[i].l,(unsigned)log.call[i].q,(unsigned)log.call[i].cadr,(unsigned)log.call[i].bbcon);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;}
