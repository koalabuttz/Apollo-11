/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "irq_platform.h"
#include "interpreter_native_check.h"
#define MAIL ((volatile uint8_t *)0xc040)
static in_run run;
static executive before;
static const exec_request high={04000,{IN_HIGH_ENTRY,IN_HIGH_BANK},1};
static unsigned failures,caseno,stepno;
static uint8_t reason;
static void check(int good) {
    if(!good) {
        if(failures==0) { MAIL[7]=(uint8_t)caseno; MAIL[8]=(uint8_t)stepno; MAIL[9]=reason; }
        ++failures;
    }
}
int main(void) {
    unsigned n,step;
    uint8_t token;
    MAIL[0]=0; MAIL[7]=0; MAIL[8]=0; MAIL[9]=0; failures=0;
    puts("APOLLO MIXED NATIVE CONTEXTS");
    memset(c64_irq_stack,0xcd,256);
    for(n=0;n<IN_CASE_COUNT;++n) {
        caseno=n; in_run_init(&run,n);
        run.runtime.exclusion=&c64_exclusion;
        run.interrupt_status=c64_irq_status;
        run.expected_irq_mask=0;
        for(step=0;step<in_steps(n);++step) {
            enum exec_status status; stepno=step;
            if(n==0 && step==3) {
                token=c64_exclusion.enter(0);
                before=run.state;
                reason=1; check(c64_irq_install(&run.queue,&high,1));
                MAIL[6]=0x90;
                c64_exclusion.leave(0,token);
                while(c64_irq_done==0) __asm__ volatile("nop" : : : "memory");
                MAIL[6]=0x91;
                reason=2; check(c64_irq_count==1 && c64_irq_last_status==EXEC_OK);
                reason=3; check(memcmp(&before,&run.state,sizeof before)==0);
                reason=4; check(run.queue.read==1 && run.queue.write==2);
                c64_irq_remove();
                status=exec_native_poll(&run.runtime);
                reason=5; check(run.queue.read==2 && c64_irq_stack_fault==0);
            } else status=in_run_step(&run,step);
            reason=6; check(in_checkpoint(&run,n,step,status));
            reason=7; check((c64_irq_status()&4u)==0);
        }
    }
    /* Repeat a complete mixed sequence with caller IRQs already masked. */
    caseno=35; in_run_init(&run,0); run.runtime.exclusion=&c64_exclusion;
    run.interrupt_status=c64_irq_status; run.expected_irq_mask=4;
    token=c64_exclusion.enter(0);
    for(step=0;step<IN_STEPS;++step) {
        enum exec_status status; stepno=step; status=in_run_step(&run,step);
        reason=8; check(in_checkpoint(&run,0,step,status)); reason=9; check((c64_irq_status()&4u)!=0);
    }
    c64_exclusion.leave(0,token);
    reason=7; check((c64_irq_status()&4u)==0);
    MAIL[1]=(uint8_t)failures; MAIL[2]=(uint8_t)(failures>>8);
    MAIL[3]=140; MAIL[4]=16; MAIL[5]=c64_irq_count;
    puts(failures ? "MIXED CONTEXT FAILED" : "140 CONTEXTS + MASKED + IRQ PASS");
    MAIL[0]=0xa5;
    return failures;
}
