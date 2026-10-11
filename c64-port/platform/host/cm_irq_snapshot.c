/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_irq_cases.h"
int main(void) {
 uint16_t n; uint8_t i,j; cm_scenario s;
 for(n=0;n<CM_IRQ_CASE_COUNT;++n) {
  int status=cm_irq_case_run(&s,n,0); printf("%u %d",n,status);
  for(i=0;i<CM_EXEC_CORE_COUNT;++i) for(j=0;j<12;++j) printf(" %u",s.state.core[i][j]);
  for(i=0;i<5;++i) for(j=0;j<44;++j) printf(" %u",s.state.vac[i][j]);
  printf(" %u %u %u %u %u %u %u %u\n",s.state.newjob,s.state.newprio,s.state.newloc[0],s.state.newloc[1],s.state.locctr,s.state.fixloc,s.state.ovfind,s.state.alarm);
 } return 0;
}
