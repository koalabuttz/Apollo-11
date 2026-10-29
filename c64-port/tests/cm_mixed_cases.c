/* SPDX-License-Identifier: MIT; constructors are inputs, not expected results. */
#include "cm_mixed_cases.h"
void cm_mixed_prepare(cm_executive *s,unsigned n) {
    unsigned i,j;
    for(i=0;i<CM_EXEC_CORE_COUNT;++i) {
        for(j=0;j<CM_EXEC_CORE_WORDS;++j)
            s->core[i][j]=(agc_word)((n*617u+i*83u+j*117u)&AGC_MASK);
        s->core[i][CM_EXEC_PRIORITY]=AGC_NEG_ZERO;
    }
    for(i=1;i<=n && n<6u;++i) {
        s->core[i][CM_EXEC_PRIORITY]=(agc_word)(AGC_MASK^03110u);
        s->core[i][CM_EXEC_LOC]=(agc_word)(012300u+i);
        s->core[i][CM_EXEC_PUSH]=CM_EXEC_FAKEPRET;
    }
    for(i=0;i<CM_EXEC_VAC_COUNT;++i) {
        for(j=0;j<CM_EXEC_VAC_WORDS;++j)
            s->vac[i][j]=(agc_word)((n*211u+i*19u+j*71u)&AGC_MASK);
        s->vac[i][0]=(agc_word)(0400u+i*44u);
    }
    s->newjob=AGC_NEG_ZERO; s->newprio=0;
    s->newloc[0]=0; s->newloc[1]=0; s->locctr=0;
    s->fixloc=0; s->ovfind=0; s->alarm=0;
    if(n>=7u) {
        unsigned slot=1u+(n%6u);
        for(i=0;i<CM_EXEC_CORE_COUNT;++i) s->core[i][CM_EXEC_PRIORITY]=AGC_NEG_ZERO;
        s->core[0][CM_EXEC_PRIORITY]=03401; s->core[0][CM_EXEC_LOC]=03333;
        s->core[0][CM_EXEC_BANK]=040003; s->core[0][CM_EXEC_PUSH]=0401;
        s->fixloc=0401; s->vac[0][0]=0; s->vac[1][0]=0;
        for(i=0;i<7;++i) s->core[slot][i]=(agc_word)(0100u+i);
        s->core[slot][CM_EXEC_MODE]=1;
        s->core[slot][CM_EXEC_LOC]=(agc_word)(AGC_MASK^(IN_LOW_B-1u));
        s->core[slot][CM_EXEC_BANK]=IN_LOW_BANK;
        s->core[slot][CM_EXEC_PUSH]=(agc_word)(AGC_MASK^(0455u+6u));
        s->core[slot][CM_EXEC_PRIORITY]=02455;
        s->vac[1][2]=01007; s->newjob=(agc_word)(slot*CM_EXEC_CORE_WORDS);
        if(n>=21u && n<28u) s->core[0][CM_EXEC_PRIORITY]=AGC_NEG_ZERO;
    }
}
unsigned cm_mixed_steps(unsigned n) { return n<7u ? IN_STEPS : 1u; }
void cm_mixed_event_prepare(cm_mixed_event *e,unsigned n,unsigned step) {
    static const enum cm_mixed_operation ops[IN_STEPS]={
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
void cm_mixed_touch(cm_executive *s,uint8_t touch) {
    unsigned i,vac;
    if(touch==0) return;
    for(i=0;i<7;++i) s->core[0][i]=(agc_word)(touch*0100u+i);
    s->core[0][CM_EXEC_MODE]=(agc_word)(touch==1 ? 1 : 0);
    s->core[0][CM_EXEC_PUSH]=(agc_word)(s->fixloc+6u);
    s->ovfind=touch==1 ? 1 : AGC_NEG_ZERO;
    vac=(unsigned)((s->fixloc-0401u)/44u);
    s->vac[vac][2]=(agc_word)(touch*01000u+7u);
}
#ifndef IN_INPUT_ONLY
#include <string.h>
static enum cm_exec_action action(cm_executive *s,void *user,exec_address *resume,
                                agc_word loc,agc_word bank,enum cm_exec_action result,
                                uint8_t touch,uint8_t verify) {
    cm_mixed_run *r=user;
    if(r->frame_active || r->runtime.in_callback!=1) ++r->failures;
    r->frame_active=1; ++r->callbacks;
    if(verify) {
        if(s->core[0][CM_EXEC_MPAC]!=(agc_word)(verify*0100u) ||
           s->core[0][CM_EXEC_MODE]!=(verify==1 ? 1u : 0u) ||
           s->core[0][CM_EXEC_PUSH]!=(agc_word)(s->fixloc+6u) ||
           s->ovfind!=(verify==1 ? 1u : AGC_NEG_ZERO)) ++r->failures;
    }
    cm_mixed_touch(s,touch); resume->location=loc; resume->bank=bank;
    r->frame_active=0; return result;
}
#define BODY(name,loc,bank,result,touch,verify) \
static enum cm_exec_action name(cm_executive *s,void *u,exec_address *a) { \
    return action(s,u,a,loc,bank,result,touch,verify); }
BODY(low_entry,IN_LOW_A,IN_LOW_BANK,CM_EXEC_ENTER_INTERPRETER,0,0)
BODY(low_a,IN_LOW_B,IN_LOW_BANK,CM_EXEC_YIELD,1,0)
BODY(low_b,IN_LOW_SLEEP,IN_LOW_BANK,CM_EXEC_EXIT_INTERPRETER,0,1)
BODY(low_sleep,IN_LOW_WAKE,IN_LOW_BANK,CM_EXEC_SLEEP,0,1)
BODY(low_wake,IN_LOW_C,IN_LOW_BANK,CM_EXEC_ENTER_INTERPRETER,0,1)
BODY(low_c,IN_LOW_FINISH,IN_LOW_BANK,CM_EXEC_EXIT_INTERPRETER,0,1)
BODY(low_finish,0,IN_LOW_BANK,CM_EXEC_FINISH,0,1)
BODY(high_entry,IN_HIGH_A,IN_HIGH_BANK,CM_EXEC_ENTER_INTERPRETER,0,0)
BODY(high_a,IN_HIGH_B,IN_HIGH_BANK,CM_EXEC_YIELD,2,0)
BODY(high_b,IN_HIGH_FINISH,IN_HIGH_BANK,CM_EXEC_EXIT_INTERPRETER,0,2)
BODY(high_finish,0,IN_HIGH_BANK,CM_EXEC_FINISH,0,2)
BODY(high_bridge,IN_HIGH_FINISH,IN_HIGH_BANK,CM_EXEC_YIELD,0,0)
BODY(high_sleep,IN_HIGH_WAKE,IN_HIGH_BANK,CM_EXEC_SLEEP,0,0)
static enum cm_exec_action high_priority(cm_executive *s,void *u,exec_address *a) {
    cm_mixed_run *r=u;
    if(cm_exec_native_request_priority(&r->runtime,01000)!=EXEC_OK) ++r->failures;
    return action(s,u,a,IN_HIGH_FINISH,IN_HIGH_BANK,CM_EXEC_REPRIORITIZE,0,0);
}
static const cm_exec_context_mapping mappings[]={
    {{IN_LOW_ENTRY,IN_LOW_BANK},CM_EXEC_CONTEXT_BASIC,low_entry},
    {{IN_LOW_A,IN_LOW_BANK},CM_EXEC_CONTEXT_INTERPRETIVE,low_a},
    {{IN_LOW_B,IN_LOW_BANK},CM_EXEC_CONTEXT_INTERPRETIVE,low_b},
    {{IN_LOW_SLEEP,IN_LOW_BANK},CM_EXEC_CONTEXT_BASIC,low_sleep},
    {{IN_LOW_WAKE,IN_LOW_BANK},CM_EXEC_CONTEXT_BASIC,low_wake},
    {{IN_LOW_C,IN_LOW_BANK},CM_EXEC_CONTEXT_INTERPRETIVE,low_c},
    {{IN_LOW_FINISH,IN_LOW_BANK},CM_EXEC_CONTEXT_BASIC,low_finish},
    {{IN_HIGH_ENTRY,IN_HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high_entry},
    {{IN_HIGH_A,IN_HIGH_BANK},CM_EXEC_CONTEXT_INTERPRETIVE,high_a},
    {{IN_HIGH_B,IN_HIGH_BANK},CM_EXEC_CONTEXT_INTERPRETIVE,high_b},
    {{IN_HIGH_FINISH,IN_HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high_finish},
    {{IN_HIGH_BRIDGE,IN_HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high_bridge},
    {{IN_HIGH_SLEEP,IN_HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high_sleep},
    {{IN_HIGH_WAKE,IN_HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high_finish},
    {{IN_HIGH_PRIORITY,IN_HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high_priority},
};
void cm_mixed_run_init(cm_mixed_run *r,unsigned n) {
    memset(r,0,sizeof *r); r->number=n; cm_mixed_prepare(&r->state,n);
    if(cm_exec_native_context_init(&r->runtime,&r->state,mappings,15,r)!=EXEC_OK) ++r->failures;
    if((n>=7u && n<21u) || n>=28u) {
        r->runtime.current.location=n<14u ? IN_HIGH_BRIDGE : n<21u ? IN_HIGH_SLEEP : IN_HIGH_PRIORITY;
        r->runtime.current.bank=IN_HIGH_BANK; r->runtime.running=1;
    }
}
enum exec_status cm_mixed_run_step(cm_mixed_run *r,unsigned step) {
    cm_mixed_event e; enum exec_status status;
    cm_mixed_event_prepare(&e,r->number,step);
    if(e.operation==IN_ADMIT) {
        status=cm_exec_native_admit(&r->runtime,e.priority,e.address,1);
    } else if(e.operation==IN_START) status=cm_exec_native_start(&r->runtime);
    else if(e.operation==IN_WAKE) status=cm_exec_native_wake(&r->runtime,
        (agc_word)((e.address.location-02000u)|(e.address.bank&076000u)));
    else status=cm_exec_native_step(&r->runtime);
    if(r->frame_active || r->runtime.in_callback) ++r->failures;
    return status;
}
#endif
