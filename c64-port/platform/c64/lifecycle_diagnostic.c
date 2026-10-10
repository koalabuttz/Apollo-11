/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "irq_platform.h"
#include "lifecycle_cases.h"
#include "lifecycle_check.h"
#include "lifecycle_vectors.h"
#include "state_digest.h"
#define MAIL ((volatile uint8_t *)0xc040)
#define MARK(v) (MAIL[8]=(v))
static executive state,before;
static exec_native runtime;
static exec_irq_queue queue;
static unsigned failures;
static uint16_t trace;
static void check(int good) { if(!good) ++failures; }
static enum exec_action sleeper(executive *s,void *user,exec_address *resume) {
    (void)user; trace=1; dispatch_touch(s,1);
    resume->location=LOW_RESUME; resume->bank=LOW_BANK; return EXEC_SLEEP;
}
static enum exec_action waker(executive *s,void *user,exec_address *resume) {
    (void)user; trace=12;
    check(exec_native_wake(&runtime,06011)==EXEC_OK);
    check(exec_native_request_priority(&runtime,01000)==EXEC_OK);
    dispatch_touch(s,2);
    resume->location=HIGH_RESUME; resume->bank=HIGH_BANK; return EXEC_REPRIORITIZE;
}
static enum exec_action awakened(executive *s,void *user,exec_address *resume) {
    (void)user; (void)resume; trace=123;
    check(s->core[0][EXEC_MPAC]==0100 && s->ovfind==1);
    check(s->vac[0][0]==0 && s->vac[0][1]==01007);
    dispatch_touch(s,3); return EXEC_FINISH;
}
static enum exec_action finish(executive *s,void *user,exec_address *resume) {
    (void)user; (void)resume; trace=1234;
    check(s->core[0][EXEC_MPAC]==0200 && s->ovfind==0);
    check(s->vac[0][0]==0400); dispatch_touch(s,4); return EXEC_FINISH;
}
static const exec_mapping jobs[]={
    {{LOW_ENTRY,LOW_BANK},sleeper},{{LOW_RESUME,LOW_BANK},awakened},
    {{HIGH_ENTRY,HIGH_BANK},waker},{{HIGH_RESUME,HIGH_BANK},finish}
};
static void timing_samples(void) {
    uint8_t n,token;
    for(n=0;n<76;++n) {
        lifecycle_event e;
        exec_address next={0,0};
        enum exec_status status;
        lifecycle_prepare(&state,n); lifecycle_event_prepare(&e,n,0);
        MAIL[13]=n;
        token=c64_exclusion.enter(0); MARK(0x10);
        status=lifecycle_apply(&state,&e,&next);
        MARK(0x20); c64_exclusion.leave(0,token);
        check(dispatch_state_digest(&state,(int)status,next)==lifecycle_reference_digest[n]);
    }
}
static void irq_wakeup(void) {
    exec_request request={02000,{HIGH_ENTRY,HIGH_BANK},0};
    exec_address low={LOW_ENTRY,LOW_BANK};
    uint8_t token,installed;
    exec_init(&state); exec_irq_init(&queue);
    check(exec_native_init(&runtime,&state,jobs,4,0)==EXEC_OK);
    check(exec_native_bind_irq(&runtime,&queue,&c64_exclusion)==EXEC_OK);
    token=c64_exclusion.enter(0);
    installed=(uint8_t)c64_irq_install(&queue,&request,1); check(installed);
    if(!installed) { c64_exclusion.leave(0,token); return; }
    check(exec_native_admit(&runtime,03000,low,1)==EXEC_OK);
    check(exec_native_start(&runtime)==EXEC_OK);
    check(exec_native_step(&runtime)==EXEC_IDLE);
    check(state.core[0][EXEC_PRIORITY]==(03401 ^ AGC_MASK) && state.vac[0][0]==0);
    before=state; MARK(0x90); c64_exclusion.leave(0,token);
    while(c64_irq_done==0) __asm__ volatile("nop" : : : "memory");
    MARK(0x91);
    check(c64_irq_count==1 && c64_irq_last_status==EXEC_OK && queue.write==1 && queue.read==0);
    check(memcmp(&state,&before,sizeof state)==0);
    check(exec_native_step(&runtime)==EXEC_OK && runtime.current.location==LOW_RESUME);
    MARK(0x92); check(trace==12 && queue.read==1);
    check(exec_native_step(&runtime)==EXEC_OK && runtime.current.location==HIGH_RESUME);
    MARK(0x93);
    check(exec_native_step(&runtime)==EXEC_IDLE && trace==1234);
    MARK(0x94); c64_irq_remove();
    check(c64_irq_stack_fault==0 && (c64_irq_status() & 4u)==0);
    check(*(volatile uint16_t *)0x0314==0xea31);
}
int main(void) {
    MAIL[0]=0; failures=0;
    puts("APOLLO SLEEP / WAKE / PRIORITY");
    memset(c64_irq_stack,0xcd,256);
    failures+=lifecycle_check(&state,&c64_exclusion);
    timing_samples(); irq_wakeup();
    MAIL[1]=(uint8_t)failures; MAIL[2]=(uint8_t)(failures>>8);
    MAIL[3]=81; MAIL[4]=115; MAIL[5]=5; MAIL[6]=c64_irq_count; MAIL[7]=76;
    MAIL[9]=(uint8_t)sizeof(executive); MAIL[10]=(uint8_t)(sizeof(executive)>>8);
    MAIL[11]=(uint8_t)trace; MAIL[12]=(uint8_t)(trace>>8);
    puts(failures ? "LIFECYCLE DIAGNOSTIC FAILED" : "115 CHECKPOINTS + IRQ WAKE PASS");
    MAIL[0]=0xa5;
    return failures;
}
