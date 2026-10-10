/* SPDX-License-Identifier: MIT
 * Independent fixture INPUTS and actual native demonstration jobs. */
#include "dispatch_cases.h"
#include "scenarios.h"
static void parameters(uint8_t n, agc_word *low, agc_word *high, uint8_t *lv, uint8_t *hv) {
    uint8_t k=(uint8_t)(n-DISPATCH_SEQUENCE_FIRST);
    *low=02000; *high=03000; *lv=0; *hv=0;
    if (k==1 || k==3 || k==4 || k==5) *high=02000;
    if (k==2 || k==6) *high=01000;
    if (k==3 || k==5 || k==7) *hv=1;
    if (k==4 || k==5 || k==7) *lv=1;
    if (k==6) *low=037000;
    if (k==7) { *low=01000; *high=037000; }
}
void dispatch_prepare(executive *s, uint8_t n) {
    scenario initial;
    uint8_t i,j,slot=(uint8_t)(1u+n%7u);
    scenario_prepare(&initial,23); *s=initial.state;
    s->newloc[0]=01234; s->newloc[1]=0454; s->locctr=072;
    s->fixloc=EXEC_FAKEPRET;
    /* Retain nonzero stale words in ALL cores and VACs, including both zeros. */
    for (i=0;i<EXEC_CORE_COUNT;++i) {
        for (j=0;j<8;++j) s->core[i][j]=(agc_word)(0100+i*020+j);
        s->core[i][EXEC_MODE]=i%2u ? AGC_NEG_ZERO : 0;
        s->core[i][EXEC_LOC]=(agc_word)(03400+i);
        s->core[i][EXEC_BANK]=(agc_word)(010004+i%4u);
        s->core[i][EXEC_PUSH]=(agc_word)(EXEC_FAKEPRET+i);
    }
    for (i=0;i<EXEC_VAC_COUNT;++i)
        for (j=1;j<EXEC_VAC_WORDS;++j) s->vac[i][j]=(agc_word)(i*0100+j);
    if (n>=DISPATCH_SEQUENCE_FIRST && n<50) { s->newjob=AGC_NEG_ZERO; return; }
    s->core[0][EXEC_PRIORITY]=02401; s->core[0][EXEC_PUSH]=0420;
    s->vac[0][0]=0; s->ovfind=(agc_word)((n/7u)%2u);
    s->core[slot][EXEC_PRIORITY]=03455; s->core[slot][EXEC_PUSH]=0470;
    s->vac[1][0]=0; s->newjob=(agc_word)(slot*12u);
    if (n<28 && n>=14) s->core[slot][EXEC_PUSH]=(agc_word)(0470 ^ AGC_MASK);
    if (n>=28 && n<36) {
        s->core[slot][EXEC_PUSH]=(n & 1u) ? AGC_NEG_ZERO : 0;
        s->ovfind=n>=32 ? AGC_NEG_ZERO : (agc_word)(n & 2u);
    }
    if (n>=36 && n<42) {
        for (i=1;i<EXEC_CORE_COUNT;++i) s->core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
        s->vac[1][0]=0454;
        s->ovfind=1; s->newjob=0;
        s->core[1][EXEC_PRIORITY]=03110;
        s->core[1][EXEC_PUSH]=(agc_word)(EXEC_FAKEPRET ^ AGC_MASK);
        if (n==37) { s->core[2][EXEC_PRIORITY]=03110;
                     s->core[2][EXEC_PUSH]=EXEC_FAKEPRET; } /* exact NOVAC tie */
        if (n==38 || n==39 || n==40) {
            s->core[7][EXEC_PRIORITY]=n==38 ? 037531 : 03531;
            s->core[7][EXEC_PUSH]=0550; s->vac[2][0]=0;
        }
        if (n==39) s->core[1][EXEC_PRIORITY]=(agc_word)(03110 ^ AGC_MASK);
        /* Case 40: equal upper priority, VAC low bits beat FAKEPRET. */
        if (n==41) s->core[1][EXEC_PRIORITY]=AGC_NEG_ZERO;
    }
    if (n>=50) {
        s->newjob=n==50 ? 0 : (agc_word)(slot*12u); s->ovfind=0;
        if (n==50) { s->core[slot][EXEC_PRIORITY]=AGC_NEG_ZERO; s->vac[1][0]=0454; }
    }
}
uint8_t dispatch_steps(uint8_t n) { return n>=DISPATCH_SEQUENCE_FIRST && n<50 ? 8 : 1; }
uint8_t dispatch_vector_index(uint8_t n, uint8_t step) {
    return n<42 ? n : n<50 ? (uint8_t)(42u+(n-42u)*8u+step) : (uint8_t)(106u+n-50u);
}
void dispatch_event_prepare(dispatch_event *e, uint8_t n, uint8_t step) {
    agc_word low,high;
    uint8_t lv,hv;
    int switches;
    e->address.location=02400; e->address.bank=06122;
    e->priority=0; e->needs_vac=0; e->superbank=0120; e->touch=0;
    e->operation=n<36 ? DISPATCH_YIELD : DISPATCH_COMPLETE;
    if (n>=50) { e->operation=DISPATCH_START; return; }
    if (n<DISPATCH_SEQUENCE_FIRST) return;
    parameters(n,&low,&high,&lv,&hv);
    switches=(high+(hv ? (lv ? 0455 : 0401) : EXEC_FAKEPRET)) >
             (low+(lv ? 0401 : EXEC_FAKEPRET));
    if (step==0 || step==2) {
        e->operation=DISPATCH_ADMIT;
        e->priority=step==0 ? low : high; e->needs_vac=step==0 ? lv : hv;
        e->address.location=step==0 ? LOW_ENTRY : HIGH_ENTRY;
        e->address.bank=step==0 ? LOW_BANK : HIGH_BANK;
    } else if (step==1) e->operation=DISPATCH_START;
    else if (step==3) {
        e->operation=DISPATCH_YIELD; e->touch=1;
        e->address.location=LOW_RESUME; e->address.bank=LOW_BANK;
    } else if (step==4 || step==6) {
        int high_entry=(step==4) == switches;
        e->operation=DISPATCH_YIELD; e->touch=high_entry ? 2 : 3;
        e->address.location=high_entry ? HIGH_RESUME : LOW_FINISH;
        e->address.bank=high_entry ? HIGH_BANK : LOW_BANK;
    } else {
        int high_finish=(step==5) == switches;
        e->operation=DISPATCH_COMPLETE; e->touch=high_finish ? 4 : 5;
        e->superbank=(agc_word)((high_finish ? HIGH_BANK : LOW_BANK) & 0160);
    }
}
void dispatch_touch(executive *s, uint8_t pattern) {
    agc_word base;
    uint8_t i;
    if (pattern==0) return;
    for (i=0;i<8;++i) s->core[0][i]=(agc_word)(pattern*0100u+i);
    s->core[0][EXEC_MODE]=(pattern & 1u) ? AGC_NEG_ZERO : 0;
    s->ovfind=(agc_word)(pattern & 1u);
    base=(agc_word)(s->core[0][EXEC_PRIORITY] & EXEC_LOW9);
    for (i=0;i<EXEC_VAC_COUNT;++i)
        if (base == (agc_word)(EXEC_VAC1USE+i*EXEC_VAC_WORDS+1))
            s->vac[i][1]=(agc_word)(pattern*01000u+7u);
}
#ifndef DISPATCH_INPUT_ONLY
enum exec_status dispatch_apply(executive *s, const dispatch_event *e, exec_address *next) {
    dispatch_touch(s,e->touch);
    if (e->operation==DISPATCH_ADMIT) {
        next->location=0; next->bank=0;
        return exec_admit(s,e->priority,e->address.location,e->address.bank,e->needs_vac);
    }
    if (e->operation==DISPATCH_START) return exec_start_basic(s,next);
    if (e->operation==DISPATCH_COMPLETE) return exec_complete_basic(s,e->superbank,next);
    if (s->newjob==0) { *next=e->address; return EXEC_OK; }
    return exec_change_basic(s,e->address,next);
}
typedef struct {
    dispatch_observer observe;
    void *user;
    uint8_t n,step,failed;
} demo;
static enum exec_action low_entry(executive *s, void *user, exec_address *resume) {
    demo *d=(demo *)user;
    dispatch_event e;
    exec_address zero={0,0};
    enum exec_status status;
    dispatch_event_prepare(&e,d->n,2);
    status=exec_admit(s,e.priority,HIGH_ENTRY,HIGH_BANK,e.needs_vac);
    if (status!=EXEC_OK) d->failed=1;
    if (d->observe != 0) d->observe(d->user,d->step,s,status,zero);
    ++d->step;
    dispatch_touch(s,1); resume->location=LOW_RESUME; resume->bank=LOW_BANK;
    return EXEC_YIELD;
}
static void payload(executive *s,demo *d,uint8_t pattern) {
    uint8_t i;
    agc_word base=(agc_word)(s->core[0][EXEC_PRIORITY] & EXEC_LOW9);
    for (i=0;i<7;++i)
        if (s->core[0][i]!=(agc_word)(pattern*0100u+i)) d->failed=1;
    if (s->core[0][EXEC_MODE]!=((pattern & 1u) ? AGC_NEG_ZERO : 0) ||
        s->ovfind!=(agc_word)(pattern & 1u)) d->failed=1;
    for (i=0;i<EXEC_VAC_COUNT;++i)
        if (base==(agc_word)(EXEC_VAC1USE+i*EXEC_VAC_WORDS+1) &&
            s->vac[i][1]!=(agc_word)(pattern*01000u+7u)) d->failed=1;
}
static enum exec_action high_entry(executive *s, void *user, exec_address *resume) {
    (void)user; dispatch_touch(s,2);
    resume->location=HIGH_RESUME; resume->bank=HIGH_BANK; return EXEC_YIELD;
}
static enum exec_action low_resume(executive *s, void *user, exec_address *resume) {
    payload(s,(demo *)user,1); dispatch_touch(s,3);
    resume->location=LOW_FINISH; resume->bank=LOW_BANK; return EXEC_YIELD;
}
static enum exec_action high_finish(executive *s, void *user, exec_address *resume) {
    payload(s,(demo *)user,2); (void)resume; dispatch_touch(s,4); return EXEC_FINISH;
}
static enum exec_action low_finish(executive *s, void *user, exec_address *resume) {
    payload(s,(demo *)user,3); (void)resume; dispatch_touch(s,5); return EXEC_FINISH;
}
static const exec_mapping jobs[]={
    {{LOW_ENTRY,LOW_BANK},low_entry}, {{LOW_RESUME,LOW_BANK},low_resume},
    {{LOW_FINISH,LOW_BANK},low_finish}, {{HIGH_ENTRY,HIGH_BANK},high_entry},
    {{HIGH_RESUME,HIGH_BANK},high_finish}
};
int dispatch_demo(executive *s, uint8_t n, dispatch_observer observe, void *user) {
    exec_native runtime;
    dispatch_event e;
    exec_address zero={0,0};
    enum exec_status status;
    demo d;
    d.observe=observe; d.user=user; d.n=n; d.step=0; d.failed=0;
    dispatch_prepare(s,n); dispatch_event_prepare(&e,n,0);
    status=exec_admit(s,e.priority,LOW_ENTRY,LOW_BANK,e.needs_vac);
    if (observe != 0) observe(user,d.step,s,status,zero);
    ++d.step;
    if (status!=EXEC_OK || exec_native_init(&runtime,s,jobs,5,&d)!=EXEC_OK) return 0;
    status=exec_native_start(&runtime);
    if (observe != 0) observe(user,d.step,s,status,runtime.current);
    ++d.step;
    if (status!=EXEC_OK) return 0;
    while (runtime.running != 0 && d.step<9) {
        status=exec_native_step(&runtime);
        if (observe != 0) observe(user,d.step,s,status,runtime.current);
        ++d.step;
        if (status!=EXEC_OK && status!=EXEC_IDLE) return 0;
    }
    return d.step==8 && d.failed==0 && status==EXEC_IDLE;
}

#endif
