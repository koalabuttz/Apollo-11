/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lifecycle_cases.h"
static uint8_t case_number;
static void row(void *user,uint8_t step,const executive *s,enum exec_status status,exec_address next) {
    uint8_t i,j;
    (void)user;
    printf("%u %u %d",case_number,step,(int)status);
    for(i=0;i<8;++i) for(j=0;j<12;++j) printf(" %u",s->core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) printf(" %u",s->vac[i][j]);
    printf(" %u %u %u %u %u %u %u %u %u %u\n",s->newjob,s->newprio,
           s->newloc[0],s->newloc[1],s->locctr,s->fixloc,s->ovfind,s->alarm,next.location,next.bank);
}
int main(void) {
    executive state;
    for(case_number=0;case_number<LIFECYCLE_CASE_COUNT;++case_number) {
        if(case_number>=76) {
            if(!lifecycle_demo(&state,case_number,row,0,0)) return 1;
        } else {
            lifecycle_event e;
            exec_address next={0,0};
            enum exec_status status;
            lifecycle_prepare(&state,case_number);
            lifecycle_event_prepare(&e,case_number,0);
            status=lifecycle_apply(&state,&e,&next);
            row(0,0,&state,status,next);
        }
    }
    return 0;
}
