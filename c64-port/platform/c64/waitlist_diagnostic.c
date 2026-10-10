/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "timer_platform.h"
#include "waitlist_check.h"
#include "waitlist_vectors.h"
#define REG(a) (*(volatile uint8_t *)(a))
#define MAIL ((volatile uint8_t *)0xc050)
#define MARK(v) (MAIL[8]=(v))
static waitlist tasks,task_before;
static executive jobs,job_before;
static wl_native task_runtime;
static exec_native job_runtime;
static exec_irq_queue requests;
static wl_clock clock;
static unsigned failures;
static uint16_t trace;
static void check(int good) { if(!good) ++failures; }
static enum wl_status noop(wl_native *r,void *user) { (void)r; (void)user; check((c64_timer_status() & 4u)!=0); return WL_OK; }
static enum exec_action sleeper(executive *s,void *user,exec_address *resume) {
    (void)user; trace=1;
    s->core[0][EXEC_MPAC]=012345; s->ovfind=1; s->vac[0][1]=07654;
    resume->location=02001; resume->bank=06003; return EXEC_SLEEP;
}
static enum exec_action awakened(executive *s,void *user,exec_address *resume) {
    (void)user; (void)resume; check(trace==3); trace=4;
    check(s->core[0][EXEC_MPAC]==012345 && s->ovfind==1 && s->vac[0][0]==0 && s->vac[0][1]==07654);
    return EXEC_FINISH;
}
static enum exec_action worker(executive *s,void *user,exec_address *resume) {
    (void)user; (void)resume; check(trace==4); trace=5; check(s->vac[0][0]==0400); return EXEC_FINISH;
}
static enum wl_status waking_task(wl_native *r,void *user) {
    exec_address address={02010,040060}; (void)r; (void)user;
    check((c64_timer_status() & 4u)!=0 && trace==1); trace=2;
    check(exec_native_admit(&job_runtime,02000,address,0)==EXEC_OK);
    check(job_runtime.running==0); /* Task admission never executes a job callback. */
    check(exec_native_wake(&job_runtime,06001)==EXEC_OK); trace=3;
    return WL_OK;
}
static const exec_mapping job_mapping[]={
    {{02000,06003},sleeper},{{02001,06003},awakened},{{02010,040060},worker}
};
static const wl_mapping task_mapping[]={ {{03000,022063},waking_task},{{03001,022063},noop} };
static void samples(void) {
    uint16_t n;
    for(n=0;n<156;++n) {
        uint8_t token; wait_event e; exec_address next={0,0}; enum wl_status status;
        wait_prepare(&tasks,n); wait_event_prepare(&e,n,0); MAIL[13]=(uint8_t)n;
        token=c64_timer_exclusion.enter(0); MARK(0x10);
        status=wait_apply(&tasks,&e,&next);
        MARK(0x20); c64_timer_exclusion.leave(0,token);
        check(wait_digest(&tasks,status,next)==waitlist_reference_digest[n]);
    }
}
static void source_checks(void) {
    uint8_t installed,token,old_jiffy;
    volatile uint8_t frame[16];
    uint32_t accum=0,expected=0;
    uint16_t count=0;
    wl_clock_init(&clock); memset(c64_timer_stack,0xcd,256);
    token=c64_timer_exclusion.enter(0);
    installed=(uint8_t)c64_timer_install(&clock,19705,2,1); check(installed);
    if(!installed) { c64_timer_exclusion.leave(0,token); return; }
    c64_timer_poison=1;
    check(c64_timer_register_probe()==0);
    check(clock.ticks==2 && c64_timer_count==1 && c64_timer_stack_fault==0);
    c64_timer_remove(); c64_timer_exclusion.leave(0,token);
    wl_init(&tasks); exec_init(&jobs); task_before=tasks; job_before=jobs;
    wl_clock_init(&clock); old_jiffy=REG(0x00a2);
    for(count=0;count<16;++count) frame[count]=(uint8_t)(count ^ 0x69u);
    count=0; MARK(0x90);
    installed=(uint8_t)c64_timer_install(&clock,19705,2,4); check(installed);
    if(!installed) return;
    while(c64_timer_done==0) {
        accum+=(uint32_t)count*3u+7u; ++count;
        __asm__ volatile("nop" : : : "memory");
    }
    MARK(0x91);
    while(count!=0) { --count; expected+=(uint32_t)count*3u+7u; }
    check(accum==expected);
    for(count=0;count<16;++count) check(frame[count]==(uint8_t)(count ^ 0x69u));
    check(clock.ticks==8 && clock.overflow==0 && c64_timer_count==4 && c64_timer_fault==0 && c64_timer_stack_fault==0);
    check(memcmp(&tasks,&task_before,sizeof tasks)==0 && memcmp(&jobs,&job_before,sizeof jobs)==0);
    check(REG(0x00a2)!=old_jiffy);
    c64_timer_remove();
    check(REG(0xdc0f)==8 && REG(0xdc06)==0xff && REG(0xdc07)==0xff);
}
static void task_bridge(void) {
    exec_address low={02000,06003},task={03000,022063};
    uint8_t token,installed;
    wl_init(&tasks); exec_init(&jobs); exec_irq_init(&requests); wl_clock_init(&clock);
    check(wl_native_init(&task_runtime,&tasks,task_mapping,2,0,&c64_timer_exclusion)==WL_OK);
    /* Finish the dummy slot's timer housekeeping, excluding SVCT3's IMU body. */
    check(wl_native_advance(&task_runtime,1)==WL_OK && wl_native_step(&task_runtime)==WL_IDLE);
    check(exec_native_init(&job_runtime,&jobs,job_mapping,3,0)==EXEC_OK);
    check(exec_native_bind_irq(&job_runtime,&requests,&c64_timer_exclusion)==EXEC_OK);
    check(exec_native_admit(&job_runtime,03000,low,1)==EXEC_OK && exec_native_start(&job_runtime)==EXEC_OK);
    check(exec_native_step(&job_runtime)==EXEC_IDLE && trace==1);
    check(wl_native_schedule(&task_runtime,1,task)==WL_OK);
    job_before=jobs; task_before=tasks;
    token=c64_timer_exclusion.enter(0);
    installed=(uint8_t)c64_timer_install(&clock,19705,2,1); check(installed);
    if(!installed) { c64_timer_exclusion.leave(0,token); return; }
    MARK(0x92); c64_timer_exclusion.leave(0,token);
    while(c64_timer_done==0) __asm__ volatile("nop" : : : "memory");
    MARK(0x93);
    check(clock.ticks==2 && memcmp(&jobs,&job_before,sizeof jobs)==0 && memcmp(&tasks,&task_before,sizeof tasks)==0);
    check(wl_native_collect(&task_runtime,&clock)==WL_OK && clock.ticks==0);
    check(wl_native_start(&task_runtime)==WL_OK); MARK(0x94);
    check(wl_native_step(&task_runtime)==WL_IDLE && trace==3);
    MARK(0x95);
    check(exec_native_step(&job_runtime)==EXEC_OK && trace==4);
    check(exec_native_step(&job_runtime)==EXEC_IDLE && trace==5);
    MARK(0x96); c64_timer_remove();
    check((c64_timer_status() & 4u)==0 && c64_timer_fault==0 && c64_timer_stack_fault==0);
    check(*(volatile uint16_t *)0x0314==0xea31);
}
int main(void) {
    MAIL[0]=0; failures=0;
    puts("APOLLO WAITLIST / CIA TIMER");
    failures+=waitlist_check(&tasks,&c64_timer_exclusion);
    samples(); source_checks(); task_bridge();
    MAIL[1]=(uint8_t)failures; MAIL[2]=(uint8_t)(failures>>8);
    MAIL[3]=196; MAIL[4]=(uint8_t)306; MAIL[5]=(uint8_t)(306>>8);
    MAIL[6]=10; MAIL[7]=6;
    MAIL[9]=(uint8_t)sizeof(waitlist); MAIL[10]=(uint8_t)(sizeof(waitlist)>>8);
    MAIL[11]=(uint8_t)trace; MAIL[12]=(uint8_t)(trace>>8);
    puts(failures ? "WAITLIST DIAGNOSTIC FAILED" : "306 CHECKPOINTS + CIA TASK PASS");
    MAIL[0]=0xa5; return failures;
}
