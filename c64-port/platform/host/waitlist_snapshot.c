/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "waitlist_cases.h"
static void row(void *user,uint16_t n,uint8_t step,const waitlist *s,enum wl_status status,exec_address next) {
    uint8_t i;
    (void)user;
    printf("%u %u %d %u",n,step,(int)status,s->time3);
    for(i=0;i<8;++i) printf(" %u",s->interval[i]);
    for(i=0;i<9;++i) printf(" %u %u",s->task[i].location,s->task[i].bank);
    printf(" %u %u %u %u\n",s->ruptagn,s->alarm,next.location,next.bank);
}
int main(void) {
    uint16_t n;
    for(n=0;n<WAIT_CASE_COUNT;++n) {
        uint8_t step;
        waitlist s;
        if(n>=156 && n<166) { if(wait_native_run(&s,n,0,row,0)!=0) return 2; continue; }
        wait_prepare(&s,n);
        for(step=0;step<wait_steps(n);++step) {
            exec_address next={0,0}; wait_event e; enum wl_status status;
            wait_event_prepare(&e,n,step); status=wait_apply(&s,&e,&next);
            row(0,n,step,&s,status,next);
        }
    }
    return 0;
}
