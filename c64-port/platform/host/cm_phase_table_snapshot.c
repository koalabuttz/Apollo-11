/* SPDX-License-Identifier: MIT */
/* Native cm-phase-table cases in the oracle's format (tools/cm_phase_table_reference.c). */
#include <stdio.h>
#include "cm_phase_table_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<CM_PT_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_pt_log log;uint16_t steps;
  if(cm_pt_run(&s,&io,(uint16_t)n,&log,&steps)!=EXEC_OK){fprintf(stderr,"CM phase table %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u",n,(unsigned)io.interrupt_enabled,(unsigned)log.calls);
  if(log.calls)printf(" %u %u %u %u %u",(unsigned)log.a,(unsigned)log.l,(unsigned)log.q,(unsigned)log.cadr,(unsigned)log.bbcon);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;}
