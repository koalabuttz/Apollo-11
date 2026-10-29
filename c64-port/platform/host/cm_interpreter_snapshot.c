/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_interpreter_cases.h"
void cm_interpreter_row(unsigned n,const cm_interpreter_case *c,enum exec_status status,exec_transfer next) {
 unsigned i,j;
 printf("%u %d",n,(int)status);
 for(i=0;i<7;++i) for(j=0;j<12;++j) printf(" %u",c->state.core[i][j]);
 for(i=0;i<5;++i) for(j=0;j<44;++j) printf(" %u",c->state.vac[i][j]);
 printf(" %u %u %u %u %u %u %u %u %u %u %u %u %u\n",c->state.newjob,c->state.newprio,c->state.newloc[0],c->state.newloc[1],c->state.locctr,c->state.fixloc,c->state.ovfind,c->state.alarm,c->control.intbit15,c->control.edop,next.address.location,next.address.bank,next.interpretive);
}
#ifndef CM_INTERPRETER_ROW_ONLY
int main(void) {
 unsigned n;
 for(n=0;n<CM_INTERPRETER_COUNT;++n) {
  cm_interpreter_case c; exec_transfer next={{0,0},0}; enum exec_status status;
  cm_interpreter_prepare(&c,n);
  status=cm_exec_change_interpretive(&c.state,c.superbank,&c.control,&next);
  cm_interpreter_row(n,&c,status,next);
 }
 return 0;
}
#endif
