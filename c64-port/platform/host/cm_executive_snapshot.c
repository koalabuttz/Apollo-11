/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_executive_cases.h"
int main(void) {
    uint16_t n; uint8_t i,j;
    cm_scenario s;
    for(n=0;n<CM_EXEC_CASE_COUNT;++n) {
        int r;
        cm_case_prepare(&s,n);
        r=s.complete ? cm_exec_endjob_select(&s.state) :
            (int)cm_exec_admit(&s.state,s.priority,s.location,s.bank,s.needs_vac);
        printf("%u %d",n,r);
        for(i=0;i<CM_EXEC_CORE_COUNT;++i)
            for(j=0;j<CM_EXEC_CORE_WORDS;++j) printf(" %u",s.state.core[i][j]);
        for(i=0;i<CM_EXEC_VAC_COUNT;++i)
            for(j=0;j<CM_EXEC_VAC_WORDS;++j) printf(" %u",s.state.vac[i][j]);
        printf(" %u %u %u %u %u %u %u %u\n",s.state.newjob,s.state.newprio,
               s.state.newloc[0],s.state.newloc[1],s.state.locctr,
               s.state.fixloc,s.state.ovfind,s.state.alarm);
    }
    return 0;
}
