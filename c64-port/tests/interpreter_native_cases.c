/* SPDX-License-Identifier: MIT; constructors are inputs, not expected results. */
#include "interpreter_native_cases.h"
void in_prepare(executive *s,unsigned n) {
    unsigned i,j;
    for(i=0;i<EXEC_CORE_COUNT;++i) {
        for(j=0;j<EXEC_CORE_WORDS;++j)
            s->core[i][j]=(agc_word)((n*617u+i*83u+j*117u)&AGC_MASK);
        s->core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
    }
    for(i=1;i<=n && n<7u;++i) {
        s->core[i][EXEC_PRIORITY]=(agc_word)(AGC_MASK^03110u);
        s->core[i][EXEC_LOC]=(agc_word)(012300u+i);
        s->core[i][EXEC_PUSH]=EXEC_FAKEPRET;
    }
    for(i=0;i<EXEC_VAC_COUNT;++i) {
        for(j=0;j<EXEC_VAC_WORDS;++j)
            s->vac[i][j]=(agc_word)((n*211u+i*19u+j*71u)&AGC_MASK);
        s->vac[i][0]=(agc_word)(0400u+i*44u);
    }
    s->newjob=AGC_NEG_ZERO; s->newprio=0;
    s->newloc[0]=0; s->newloc[1]=0; s->locctr=0;
    s->fixloc=0; s->ovfind=0; s->alarm=0;
    if(n>=7u) {
        unsigned slot=1u+(n%7u);
        for(i=0;i<EXEC_CORE_COUNT;++i) s->core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
        s->core[0][EXEC_PRIORITY]=03401; s->core[0][EXEC_LOC]=03333;
        s->core[0][EXEC_BANK]=040003; s->core[0][EXEC_PUSH]=0401;
        s->fixloc=0401; s->vac[0][0]=0; s->vac[1][0]=0;
        for(i=0;i<7;++i) s->core[slot][i]=(agc_word)(0100u+i);
        s->core[slot][EXEC_MODE]=1;
        s->core[slot][EXEC_LOC]=(agc_word)(AGC_MASK^(IN_LOW_B-1u));
        s->core[slot][EXEC_BANK]=IN_LOW_BANK;
        s->core[slot][EXEC_PUSH]=(agc_word)(AGC_MASK^(0455u+6u));
        s->core[slot][EXEC_PRIORITY]=02455;
        s->vac[1][2]=01007; s->newjob=(agc_word)(slot*EXEC_CORE_WORDS);
        if(n>=21u && n<28u) s->core[0][EXEC_PRIORITY]=AGC_NEG_ZERO;
    }
}
unsigned in_steps(unsigned n) { return n<7u ? IN_STEPS : 1u; }
void in_event_prepare(in_event *e,unsigned n,unsigned step) {
    static const enum in_operation ops[IN_STEPS]={
        IN_ADMIT,IN_START,IN_ENTER,IN_ADMIT,IN_YIELD,IN_ENTER,IN_YIELD,IN_EXIT,
        IN_FINISH,IN_EXIT,IN_SLEEP,IN_WAKE,IN_START,IN_ENTER,IN_EXIT,IN_FINISH};
    static const agc_word loc[IN_STEPS]={
        IN_LOW_ENTRY,0,IN_LOW_A,IN_HIGH_ENTRY,IN_LOW_B,IN_HIGH_A,IN_HIGH_B,
        IN_HIGH_FINISH,0,IN_LOW_SLEEP,IN_LOW_WAKE,IN_LOW_WAKE,0,IN_LOW_C,IN_LOW_FINISH,0};
    e->operation=ops[step]; e->address.location=loc[step];
    e->address.bank=(step==3 || (step>=5 && step<=8)) ? IN_HIGH_BANK : IN_LOW_BANK;
    e->priority=step==3 ? 04000 : 02000;
    e->touch=(uint8_t)(step==4 ? 1 : step==6 ? 2 : 0);
    if(n>=7u) {
        e->operation=n<14u ? IN_YIELD : n<21u ? IN_SLEEP : n<28u ? IN_START : IN_PRIO;
        e->address.location=n<14u ? IN_HIGH_FINISH : n<21u ? IN_HIGH_WAKE : n<28u ? 0 : IN_HIGH_FINISH;
        if(n>=28u) e->priority=01000;
        e->address.bank=IN_HIGH_BANK; e->touch=0;
    }
}
void in_touch(executive *s,uint8_t touch) {
    unsigned i,vac;
    if(touch==0) return;
    for(i=0;i<7;++i) s->core[0][i]=(agc_word)(touch*0100u+i);
    s->core[0][EXEC_MODE]=(agc_word)(touch==1 ? 1 : 0);
    s->core[0][EXEC_PUSH]=(agc_word)(s->fixloc+6u);
    s->ovfind=touch==1 ? 1 : AGC_NEG_ZERO;
    vac=(unsigned)((s->fixloc-0401u)/44u);
    s->vac[vac][2]=(agc_word)(touch*01000u+7u);
}
#ifndef IN_INPUT_ONLY
#include <string.h>
static uint8_t enter(void *user) {
    in_run *r=user; uint8_t old=r->irq_enabled; r->irq_enabled=0; return old;
}
static void leave(void *user,uint8_t old) { ((in_run *)user)->irq_enabled=old; }
static enum exec_action action(executive *s,void *user,exec_address *resume,
                                agc_word loc,agc_word bank,enum exec_action result,
                                uint8_t touch,uint8_t verify) {
    in_run *r=user;
    if(r->frame_active || r->runtime.in_callback!=1 || r->irq_enabled!=1) ++r->failures;
    if(r->interrupt_status && (r->interrupt_status()&4u)!=r->expected_irq_mask) ++r->failures;
    r->frame_active=1; ++r->callbacks;
    if(verify) {
        if(s->core[0][EXEC_MPAC]!=(agc_word)(verify*0100u) ||
           s->core[0][EXEC_MODE]!=(verify==1 ? 1u : 0u) ||
           s->core[0][EXEC_PUSH]!=(agc_word)(s->fixloc+6u) ||
           s->ovfind!=(verify==1 ? 1u : AGC_NEG_ZERO)) ++r->failures;
    }
    in_touch(s,touch); resume->location=loc; resume->bank=bank;
    r->frame_active=0; return result;
}
#define BODY(name,loc,bank,result,touch,verify) \
static enum exec_action name(executive *s,void *u,exec_address *a) { \
    return action(s,u,a,loc,bank,result,touch,verify); }
BODY(low_entry,IN_LOW_A,IN_LOW_BANK,EXEC_ENTER_INTERPRETER,0,0)
BODY(low_a,IN_LOW_B,IN_LOW_BANK,EXEC_YIELD,1,0)
BODY(low_b,IN_LOW_SLEEP,IN_LOW_BANK,EXEC_EXIT_INTERPRETER,0,1)
BODY(low_sleep,IN_LOW_WAKE,IN_LOW_BANK,EXEC_SLEEP,0,1)
BODY(low_wake,IN_LOW_C,IN_LOW_BANK,EXEC_ENTER_INTERPRETER,0,1)
BODY(low_c,IN_LOW_FINISH,IN_LOW_BANK,EXEC_EXIT_INTERPRETER,0,1)
BODY(low_finish,0,IN_LOW_BANK,EXEC_FINISH,0,1)
BODY(high_entry,IN_HIGH_A,IN_HIGH_BANK,EXEC_ENTER_INTERPRETER,0,0)
BODY(high_a,IN_HIGH_B,IN_HIGH_BANK,EXEC_YIELD,2,0)
BODY(high_b,IN_HIGH_FINISH,IN_HIGH_BANK,EXEC_EXIT_INTERPRETER,0,2)
BODY(high_finish,0,IN_HIGH_BANK,EXEC_FINISH,0,2)
BODY(high_bridge,IN_HIGH_FINISH,IN_HIGH_BANK,EXEC_YIELD,0,0)
BODY(high_sleep,IN_HIGH_WAKE,IN_HIGH_BANK,EXEC_SLEEP,0,0)
static enum exec_action high_priority(executive *s,void *u,exec_address *a) {
    in_run *r=u;
    if(exec_native_request_priority(&r->runtime,01000)!=EXEC_OK) ++r->failures;
    return action(s,u,a,IN_HIGH_FINISH,IN_HIGH_BANK,EXEC_REPRIORITIZE,0,0);
}
static const exec_context_mapping mappings[]={
    {{IN_LOW_ENTRY,IN_LOW_BANK},EXEC_CONTEXT_BASIC,low_entry},
    {{IN_LOW_A,IN_LOW_BANK},EXEC_CONTEXT_INTERPRETIVE,low_a},
    {{IN_LOW_B,IN_LOW_BANK},EXEC_CONTEXT_INTERPRETIVE,low_b},
    {{IN_LOW_SLEEP,IN_LOW_BANK},EXEC_CONTEXT_BASIC,low_sleep},
    {{IN_LOW_WAKE,IN_LOW_BANK},EXEC_CONTEXT_BASIC,low_wake},
    {{IN_LOW_C,IN_LOW_BANK},EXEC_CONTEXT_INTERPRETIVE,low_c},
    {{IN_LOW_FINISH,IN_LOW_BANK},EXEC_CONTEXT_BASIC,low_finish},
    {{IN_HIGH_ENTRY,IN_HIGH_BANK},EXEC_CONTEXT_BASIC,high_entry},
    {{IN_HIGH_A,IN_HIGH_BANK},EXEC_CONTEXT_INTERPRETIVE,high_a},
    {{IN_HIGH_B,IN_HIGH_BANK},EXEC_CONTEXT_INTERPRETIVE,high_b},
    {{IN_HIGH_FINISH,IN_HIGH_BANK},EXEC_CONTEXT_BASIC,high_finish},
    {{IN_HIGH_BRIDGE,IN_HIGH_BANK},EXEC_CONTEXT_BASIC,high_bridge},
    {{IN_HIGH_SLEEP,IN_HIGH_BANK},EXEC_CONTEXT_BASIC,high_sleep},
    {{IN_HIGH_WAKE,IN_HIGH_BANK},EXEC_CONTEXT_BASIC,high_finish},
    {{IN_HIGH_PRIORITY,IN_HIGH_BANK},EXEC_CONTEXT_BASIC,high_priority},
};
void in_run_init(in_run *r,unsigned n) {
    memset(r,0,sizeof *r); r->number=n; in_prepare(&r->state,n); r->irq_enabled=1;
    r->guard.enter=enter; r->guard.leave=leave; r->guard.user=r;
    exec_irq_init(&r->queue);
    if(exec_native_context_init(&r->runtime,&r->state,mappings,15,r)!=EXEC_OK ||
       exec_native_bind_irq(&r->runtime,&r->queue,&r->guard)!=EXEC_OK) ++r->failures;
    if((n>=7u && n<21u) || n>=28u) {
        r->runtime.current.location=n<14u ? IN_HIGH_BRIDGE : n<21u ? IN_HIGH_SLEEP : IN_HIGH_PRIORITY;
        r->runtime.current.bank=IN_HIGH_BANK; r->runtime.running=1;
    }
}
enum exec_status in_run_step(in_run *r,unsigned step) {
    in_event e; enum exec_status status;
    in_event_prepare(&e,r->number,step);
    if(e.operation==IN_ADMIT) {
        exec_request request={e.priority,e.address,1};
        status=exec_irq_post(&r->queue,&request);
        if(status==EXEC_OK) status=exec_native_poll(&r->runtime);
    } else if(e.operation==IN_START) status=exec_native_start(&r->runtime);
    else if(e.operation==IN_WAKE) status=exec_native_wake(&r->runtime,
        (agc_word)((e.address.location-02000u)|(e.address.bank&076000u)));
    else status=exec_native_step(&r->runtime);
    if(r->frame_active || r->runtime.in_callback || r->irq_enabled!=1) ++r->failures;
    if(r->interrupt_status && (r->interrupt_status()&4u)!=r->expected_irq_mask) ++r->failures;
    return status;
}
#endif
