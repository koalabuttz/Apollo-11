/* SPDX-License-Identifier: MIT */
/* Native lm-phase-table cases in the oracle's format (tools/lm_phase_table_reference.c). */
#include <stdio.h>
#include "lm_phase_table_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_PT_CASES;++n){apollo_overlay_state s;p66_bias_io io;lm_pt_log log;uint16_t steps;
  if(lm_pt_run(&s,&io,(uint16_t)n,&log,&steps)!=EXEC_OK){fprintf(stderr,"LM phase table %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u",n,(unsigned)io.interrupt_enabled,(unsigned)log.calls);
  if(log.calls)printf(" %u %u %u %u %u",(unsigned)log.a,(unsigned)log.l,(unsigned)log.q,(unsigned)log.cadr,(unsigned)log.bbcon);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;}
