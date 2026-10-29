/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "irq_platform.h"
#include "dispatch_cases.h"
#include "state_digest.h"
#include "reference_vectors.h"
#include "scenarios.h"
#define MAIL ((volatile uint8_t *)0xc020)
#define MARK(v) (MAIL[16]=(v))
static executive state,before;
static exec_irq_queue queue;
static exec_native runtime;
static unsigned failures;
static uint8_t step;
/* A byte telemetry selector also avoids the pinned SDK's register-allocation
 * failure when whole-program optimization narrows this global to LLVM i1. */
static volatile uint8_t observe;
static uint16_t trace;
static void check(int good) { if (!good) ++failures; }
static __attribute__((noinline)) void fingerprint(uint8_t stage,enum exec_status status,exec_address next) {
    if (observe==0) return;
    check(dispatch_state_digest(&state,(int)status,next)==
          dispatch_reference_digest[dispatch_vector_index(42,stage)]);
}
/* A live native C frame and 32-bit arithmetic interrupted by the C producer. */
static __attribute__((noinline)) void arithmetic(void) {
    uint8_t data[32],i;
    uint16_t k,sum=0;
    uint32_t h=UINT32_C(0x12345678);
    for (i=0;i<32;++i) data[i]=(uint8_t)(i*7u+3u);
    for (k=0;k<512;++k) {
        i=(uint8_t)(k & 31u);
        h=((h<<5)|(h>>27)) ^ ((uint32_t)data[i]+(uint32_t)k*0x101u);
        data[i]^=(uint8_t)h;
    }
    for (i=0;i<32;++i) sum=(uint16_t)(sum+data[i]);
    check(h==UINT32_C(0xd71d5ead) && sum==4545); /* Independent host calculation. */
}
/* Keep the observer work in an explicit frame in both measured/checked runs. */
static __attribute__((noinline)) void low_work(void) {
    exec_address zero={0,0};
    MAIL[17]=7; MAIL[18]=observe; MARK(0x90);
    c64_irq_arm(1);
    arithmetic();
    check(c64_irq_count==1 && c64_irq_done==0x80);
    check(queue.write==1 && queue.read==0 && state.core[1][EXEC_PRIORITY]==AGC_NEG_ZERO);
    check((c64_irq_status() & 4u)==0);
    MARK(0x94);
    check(exec_native_poll(&runtime)==EXEC_OK);
    MARK(0x95);
    fingerprint(step++,EXEC_OK,zero); /* Admission at explicit callback safe point. */
}
static enum exec_action low_entry(executive *s,void *user,exec_address *resume) {
    (void)user; trace=1;
    low_work();
    dispatch_touch(s,1);
    resume->location=LOW_RESUME; resume->bank=LOW_BANK; return EXEC_YIELD;
}
static enum exec_action high_entry(executive *s,void *user,exec_address *resume) {
    (void)user; trace=12;
    MARK(0x97); dispatch_touch(s,2);
    resume->location=HIGH_RESUME; resume->bank=HIGH_BANK; return EXEC_YIELD;
}
static enum exec_action high_finish(executive *s,void *user,exec_address *resume) {
    (void)user; (void)resume; trace=123; dispatch_touch(s,4); return EXEC_FINISH;
}
static enum exec_action low_resume(executive *s,void *user,exec_address *resume) {
    (void)user; trace=1234;
    check(s->core[0][EXEC_MPAC]==0100 && s->ovfind==1);
    dispatch_touch(s,3);
    resume->location=LOW_FINISH; resume->bank=LOW_BANK; return EXEC_YIELD;
}
static enum exec_action low_finish(executive *s,void *user,exec_address *resume) {
    (void)user; (void)resume; trace=12345;
    check(s->core[0][EXEC_MPAC]==0300 && s->ovfind==1);
    dispatch_touch(s,5); return EXEC_FINISH;
}
static const exec_mapping jobs[]={
    {{LOW_ENTRY,LOW_BANK},low_entry},{{LOW_RESUME,LOW_BANK},low_resume},
    {{LOW_FINISH,LOW_BANK},low_finish},{{HIGH_ENTRY,HIGH_BANK},high_entry},
    {{HIGH_RESUME,HIGH_BANK},high_finish}
};
static void setup(void) {
    uint8_t token=c64_exclusion.enter(0);
    exec_irq_init(&queue); dispatch_prepare(&state,42);
    check(exec_native_init(&runtime,&state,jobs,5,0)==EXEC_OK);
    check(exec_native_bind_irq(&runtime,&queue,&c64_exclusion)==EXEC_OK);
    c64_exclusion.leave(0,token);
}
static void wait_irq(void) {
    /* A bounded emulator cycle limit fails a missing IRQ; no fake completion. */
    while (c64_irq_done==0) { __asm__ volatile("nop"); }
}
static void nested_exclusion(void) {
    uint8_t outer,inner;
    check((c64_irq_status() & 4u)==0);
    outer=c64_exclusion.enter(0); inner=c64_exclusion.enter(0);
    check((c64_irq_status() & 4u)!=0);
    c64_exclusion.leave(0,inner); check((c64_irq_status() & 4u)!=0);
    c64_exclusion.leave(0,outer); check((c64_irq_status() & 4u)==0);
}
static void benchmarks(void) {
    uint8_t n,token;
    scenario fixture;
    for (n=0;n<26;++n) {
        int result;
        scenario_prepare(&fixture,n); MAIL[17]=0; MAIL[18]=n;
        token=c64_exclusion.enter(0); MARK(0x10);
        result=fixture.complete ? exec_endjob_select(&fixture.state) :
            (int)exec_admit(&fixture.state,fixture.priority,fixture.location,fixture.bank,fixture.needs_vac);
        MARK(0x20); c64_exclusion.leave(0,token);
        check(state_digest(&fixture.state,result)==reference_digest[n]);
    }
    for (n=0;n<53;++n) {
        dispatch_event e;
        exec_address next={0,0};
        enum exec_status status;
        if (n>=42 && n<50) continue;
        dispatch_prepare(&state,n); dispatch_event_prepare(&e,n,0);
        MAIL[17]=e.operation==DISPATCH_YIELD ? 1 : e.operation==DISPATCH_COMPLETE ? 2 : 3;
        MAIL[18]=n;
        token=c64_exclusion.enter(0); MARK(0x10);
        status=dispatch_apply(&state,&e,&next);
        MARK(0x20); c64_exclusion.leave(0,token);
        check(dispatch_state_digest(&state,(int)status,next)==
              dispatch_reference_digest[dispatch_vector_index(n,0)]);
    }
    /* Worst buffered batch length: four NOVAC requests, increasing priorities. */
    setup();
    for(n=0;n<4;++n) {
        exec_request request={(agc_word)(01000u+n*01000u),{HIGH_ENTRY,HIGH_BANK},0};
        check(exec_irq_post(&queue,&request)==EXEC_OK);
    }
    MAIL[17]=4; MAIL[18]=4; MARK(0x10);
    check(exec_native_poll(&runtime)==EXEC_OK);
    MARK(0x20); check(queue.read==4 && state.newjob==36);
    /* Four FINDVAC admissions fill the last four cores and scan VACs 1..4. */
    setup();
    for(n=0;n<4;++n)
        check(exec_native_admit(&runtime,01000,(exec_address){HIGH_ENTRY,HIGH_BANK},0)==EXEC_OK);
    for(n=0;n<4;++n) {
        exec_request request={(agc_word)(02000u+n*01000u),{HIGH_ENTRY,HIGH_BANK},1};
        check(exec_irq_post(&queue,&request)==EXEC_OK);
    }
    MAIL[17]=4; MAIL[18]=8; MARK(0x10);
    check(exec_native_poll(&runtime)==EXEC_OK);
    MARK(0x20); check(queue.read==4 && state.newjob==84 && state.alarm==0);
    MAIL[17]=5; MAIL[18]=0; MARK(0x10);
    token=c64_exclusion.enter(0); c64_exclusion.leave(0,token);
    MARK(0x20);
}
int main(void) {
    exec_request request={03000,{HIGH_ENTRY,HIGH_BANK},0};
    exec_address low_address={LOW_ENTRY,LOW_BANK},zero={0,0};
    uint8_t token,installed,n;
    uint32_t timer_before,timer_after;
    MAIL[0]=0; failures=0;
    puts("APOLLO BOUNDED IRQ DELIVERY");
    memset(c64_irq_stack,0xcd,256);
    nested_exclusion();
    token=c64_exclusion.enter(0); exec_irq_init(&queue);
    timer_before=((uint32_t)*(volatile uint8_t *)0xa0<<16) |
        ((uint32_t)*(volatile uint8_t *)0xa1<<8) | *(volatile uint8_t *)0xa2;
    installed=(uint8_t)c64_irq_install(&queue,&request,1);
    check(installed);
    if (!installed) { MAIL[1]=1; MAIL[0]=0xa5; return 1; }
    c64_irq_poison=1;
    MAIL[17]=6; MAIL[18]=0;
    check(c64_register_probe()==0);
    check((c64_irq_status() & 4u)!=0);
    check(c64_irq_arithmetic==0xb0 && c64_irq_count==1 && queue.write==1);
    c64_irq_poison=0; c64_exclusion.leave(0,token);
    setup(); step=0; observe=1;
    check(exec_native_admit(&runtime,02000,low_address,0)==EXEC_OK);
    fingerprint(step++,EXEC_OK,zero);
    check(exec_native_start(&runtime)==EXEC_OK);
    fingerprint(step++,EXEC_OK,runtime.current);
    while(runtime.running != 0 && step<9) {
        enum exec_status status=exec_native_step(&runtime);
        fingerprint(step++,status,runtime.current);
        if (step==4) MARK(0x96);
    }
    check(step==8 && trace==12345);
    MAIL[3]=step;
    /* Repeat the same real interrupt/yield sequence with fingerprint work
     * omitted, so latency measurements describe the native execution path. */
    setup(); step=0; observe=0;
    check(exec_native_admit(&runtime,02000,low_address,0)==EXEC_OK); ++step;
    check(exec_native_start(&runtime)==EXEC_OK); ++step;
    while(runtime.running != 0 && step<9) {
        enum exec_status status=exec_native_step(&runtime);
        check(status==EXEC_OK || status==EXEC_IDLE); ++step;
        if (step==4) MARK(0x96);
    }
    check(step==8 && trace==12345);
    /* Overflow from five actual IRQs, with a live admitted job left unchanged. */
    setup(); check(exec_native_admit(&runtime,02000,low_address,0)==EXEC_OK);
    before=state; MAIL[17]=8; MAIL[18]=5; c64_irq_arm(5); wait_irq();
    check(queue.write==4 && queue.read==0 && queue.overflow==1 && queue.rejected==1);
    check(c64_irq_last_status==EXEC_QUEUE_FULL && c64_irq_count==5);
    check(exec_native_poll(&runtime)==EXEC_QUEUE_FULL);
    check(memcmp(&state,&before,sizeof state)==0);
    /* A pending raster IRQ cannot run inside nested exclusion; publication
     * occurs only after restoring the outer caller's IRQ-enable state. */
    setup(); MAIL[17]=9; MAIL[18]=0;
    token=c64_exclusion.enter(0); c64_irq_arm(1); MARK(0x91);
    for(n=0;n<3;++n) arithmetic(); /* Longer than one PAL frame. */
    check(c64_irq_count==0 && queue.write==0);
    MARK(0x92); c64_exclusion.leave(0,token); wait_irq(); MARK(0x93);
    check(c64_irq_count==1 && queue.write==1);
    check(c64_irq_arithmetic==0x168); /* Nine binary ISR additions, including D=1 probe. */
    c64_irq_remove();
    token=c64_exclusion.enter(0);
    timer_after=((uint32_t)*(volatile uint8_t *)0xa0<<16) |
        ((uint32_t)*(volatile uint8_t *)0xa1<<8) | *(volatile uint8_t *)0xa2;
    c64_exclusion.leave(0,token);
    check(timer_after>timer_before && c64_irq_stack_fault==0);
    check((*(volatile uint16_t *)0x0314)==0xea31);
    check((c64_irq_status() & 4u)==0);
    benchmarks();
    MAIL[1]=(uint8_t)failures; MAIL[2]=(uint8_t)(failures>>8);
    MAIL[4]=9; MAIL[5]=74; MAIL[6]=(uint8_t)sizeof(exec_irq_queue);
    puts(failures ? "IRQ DIAGNOSTIC FAILED" : "IRQ REGISTERS / DELIVERY / FAULTS PASS");
    MAIL[0]=0xa5;
    return failures;
}
