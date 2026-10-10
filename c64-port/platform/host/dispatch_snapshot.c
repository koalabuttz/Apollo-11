/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dispatch_cases.h"
static uint8_t case_number;
static void row(void *user, uint8_t step, const executive *s, enum exec_status status, exec_address next) {
    uint8_t i,j;
    (void)user;
    printf("%u %u %d",case_number,step,(int)status);
    for (i=0;i<EXEC_CORE_COUNT;++i)
        for (j=0;j<EXEC_CORE_WORDS;++j) printf(" %u",s->core[i][j]);
    for (i=0;i<EXEC_VAC_COUNT;++i)
        for (j=0;j<EXEC_VAC_WORDS;++j) printf(" %u",s->vac[i][j]);
    printf(" %u %u %u %u %u %u %u %u %u %u\n",s->newjob,s->newprio,
           s->newloc[0],s->newloc[1],s->locctr,s->fixloc,s->ovfind,s->alarm,next.location,next.bank);
}
int main(void) {
    executive s;
    uint8_t step;
    for (case_number=0;case_number<DISPATCH_CASE_COUNT;++case_number) {
        if (case_number>=DISPATCH_SEQUENCE_FIRST && case_number<50) {
            if (!dispatch_demo(&s,case_number,row,0)) return 1;
        } else {
            dispatch_prepare(&s,case_number);
            for (step=0;step<dispatch_steps(case_number);++step) {
                dispatch_event e;
                exec_address next={0,0};
                enum exec_status status;
                dispatch_event_prepare(&e,case_number,step);
                status=dispatch_apply(&s,&e,&next);
                row(0,step,&s,status,next);
            }
        }
    }
    return 0;
}
