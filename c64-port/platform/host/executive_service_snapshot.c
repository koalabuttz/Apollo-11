/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "executive_service_cases.h"
void exec_service_row(unsigned n,const exec_service_case *c,
                      enum exec_status status,exec_transfer transfer) {
    unsigned i,j;
    printf("%u %u",n,(unsigned)status);
    for(i=0;i<EXEC_CORE_COUNT;++i)
        for(j=0;j<EXEC_CORE_WORDS;++j) printf(" %u",c->state.core[i][j]);
    for(i=0;i<EXEC_VAC_COUNT;++i)
        for(j=0;j<EXEC_VAC_WORDS;++j) printf(" %u",c->state.vac[i][j]);
    printf(" %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
           c->state.newjob,c->state.newprio,c->state.newloc[0],c->state.newloc[1],
           c->state.locctr,c->state.fixloc,c->state.ovfind,c->state.alarm,
           c->q,c->control.intbit15,c->control.edop,
           transfer.address.location,transfer.address.bank,transfer.interpretive);
}
#ifndef EXEC_SERVICE_ROW_ONLY
int main(void) {
    unsigned n;
    for(n=0;n<EXEC_SERVICE_CASE_COUNT;++n) {
        exec_service_case c;
        exec_transfer transfer={{0,0},0};
        enum exec_status status;
        exec_service_prepare(&c,n);
        status=n<EXEC_SERVICE_SPVAC_COUNT ? exec_spvac(&c.state,c.address,&c.q) :
            exec_change_interpretive(&c.state,c.superbank,&c.control,&transfer);
        exec_service_row(n,&c,status,transfer);
    }
    return 0;
}
#endif
