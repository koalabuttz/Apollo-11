/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "scenarios.h"
int main(void) {
    uint8_t n,i,j;
    scenario s;
    for(n=0;n<SCENARIO_COUNT;++n) {
        int r;
        scenario_prepare(&s,n);
        r=s.complete ? exec_endjob_select(&s.state) :
            (int)exec_admit(&s.state,s.priority,s.location,s.bank,s.needs_vac);
        printf("%u %d",n,r);
        for(i=0;i<EXEC_CORE_COUNT;++i)
            for(j=0;j<EXEC_CORE_WORDS;++j) printf(" %u",s.state.core[i][j]);
        for(i=0;i<EXEC_VAC_COUNT;++i)
            for(j=0;j<EXEC_VAC_WORDS;++j) printf(" %u",s.state.vac[i][j]);
        printf(" %u %u %u %u %u %u %u %u\n",s.state.newjob,s.state.newprio,
               s.state.newloc[0],s.state.newloc[1],s.state.locctr,
               s.state.fixloc,s.state.ovfind,s.state.alarm);
    }
    return 0;
}
