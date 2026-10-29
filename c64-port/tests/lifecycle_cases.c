/* SPDX-License-Identifier: MIT
 * Shared fixture INPUTS; excluded native code is never linked to the oracle. */
#include "lifecycle_cases.h"
static agc_word invert(agc_word p) { return (agc_word)(p ^ AGC_MASK); }
void lifecycle_prepare(executive *s,uint8_t n) {
    uint8_t i,slot;
    agc_word base=(n & 1u) ? 0401 : EXEC_FAKEPRET;
    dispatch_prepare(s,42);
    if (n>=76) return;
    s->core[0][EXEC_PRIORITY]=(agc_word)(03000+base);
    s->core[0][EXEC_PUSH]=(agc_word)(base+3u);
    s->core[0][EXEC_BANK]=01234; s->newjob=0; s->ovfind=(agc_word)(n & 1u);
    s->fixloc=base;
    if (base==0401) s->vac[0][0]=0;
    if (n<22) {
        slot=(uint8_t)(1u+n%7u);
        if (n<14 || n>=17) {
            s->core[slot][EXEC_PRIORITY]=02455; s->core[slot][EXEC_PUSH]=0470;
            s->vac[1][0]=0;
            if (n>=7 && n<14) s->core[slot][EXEC_PUSH]=invert(0470);
        }
        if (n==16) for(i=1;i<8;++i) {
            s->core[i][EXEC_PRIORITY]=invert(02110);
            s->core[i][EXEC_LOC]=(agc_word)(06000+i);
            s->core[i][EXEC_BANK]=0123;
        }
        if (n==17 || n==18) {
            for(i=1;i<8;++i) s->core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
            s->core[1][EXEC_PRIORITY]=02110; s->core[7][EXEC_PRIORITY]=n==17 ? 02110 : 02455;
        }
        if (n==19) s->core[slot][EXEC_PRIORITY]=037455;
        if (n==20) s->core[slot][EXEC_PUSH]=AGC_NEG_ZERO;
        if (n==21) s->ovfind=AGC_NEG_ZERO;
        return;
    }
    if (n<51) {
        /* Running NOVAC plus a dormant VAC context (or that context in core 0). */
        s->core[0][EXEC_PRIORITY]=02110; s->core[0][EXEC_PUSH]=0130;
        s->vac[0][0]=0; s->fixloc=EXEC_FAKEPRET;
        slot=n<38 ? (uint8_t)((n-22u)%8u) : 3;
        s->core[slot][EXEC_PRIORITY]=invert(n & 1u ? 01401 : 03401);
        s->core[slot][EXEC_LOC]=06234; s->core[slot][EXEC_BANK]=n & 1u ? 0167 : 022;
        s->core[slot][EXEC_PUSH]=slot==0 ? 0420 : invert(0420);
        if (slot==0) { s->newjob=AGC_NEG_ZERO; s->fixloc=0401; }
        if (n==38) s->core[slot][EXEC_LOC]=06235;
        if (n==39) {
            s->core[1][EXEC_PRIORITY]=invert(02455);
            s->core[1][EXEC_LOC]=06234; s->core[1][EXEC_BANK]=0123;
            s->vac[1][0]=0;
        }
        if (n==40 || n==41) {
            s->core[0][EXEC_PRIORITY]=invert(02401); s->core[0][EXEC_LOC]=06234;
            s->core[0][EXEC_BANK]=022; s->core[3][EXEC_PRIORITY]=AGC_NEG_ZERO;
            s->core[7][EXEC_PRIORITY]=n==40 ? 037110 : 01110; s->newjob=84;
        }
        if (n==42) s->core[slot][EXEC_PRIORITY]=invert(01401);
        if (n==43) s->core[slot][EXEC_PRIORITY]=invert(02110);
        if (n==44 || n==45) {
            s->core[7][EXEC_PRIORITY]=03110; s->newjob=84;
            s->core[slot][EXEC_PRIORITY]=invert(n==44 ? 02401 : 03401);
        }
        if (n>=47 && n<=49) s->core[slot][EXEC_LOC]=n==48 ? AGC_NEG_ZERO : 0;
        if (n==50) s->core[slot][EXEC_LOC]=076321;
        /* An active context with matching CADR must not win the wake search. */
        if (slot!=0 && n!=40 && n!=41) s->core[0][EXEC_LOC]=06234;
        return;
    }
    slot=(uint8_t)(1u+(n-51u)%7u);
    if (n<65 || n>=73) {
        s->core[slot][EXEC_PRIORITY]=02110;
        s->core[slot][EXEC_PUSH]=n & 1u ? invert(0130) : 0130;
    }
    if (n>=65 && n<=67) {
        s->core[0][EXEC_PRIORITY]=03110;
        s->core[1][EXEC_PRIORITY]=(agc_word)(02110+n-65u);
    }
    if (n==68 || n==69) {
        s->core[1][EXEC_PRIORITY]=03110; s->core[7][EXEC_PRIORITY]=n==68 ? 03110 : 03455;
        s->vac[1][0]=0;
    }
    if (n==72) for(i=1;i<8;++i) s->core[i][EXEC_PRIORITY]=invert(037110);
    if (n==73) s->core[slot][EXEC_PUSH]=AGC_NEG_ZERO;
    if (n==74) { s->core[slot][EXEC_PUSH]=0; s->ovfind=AGC_NEG_ZERO; }
    if (n==75) { s->core[7][EXEC_PRIORITY]=02110; s->core[1][EXEC_PRIORITY]=03110; s->newjob=84; }
}
uint8_t lifecycle_steps(uint8_t n) { return n<76 ? 1 : n<78 ? 8 : n<80 ? 7 : 9; }
uint8_t lifecycle_vector_index(uint8_t n,uint8_t step) {
    return n<76 ? n : n<78 ? (uint8_t)(76u+(n-76u)*8u+step) : (uint8_t)(92u+(n-78u)*7u+step);
}
void lifecycle_event_prepare(lifecycle_event *e,uint8_t n,uint8_t step) {
    e->operation=n<22 ? LIFE_SLEEP : n<51 ? LIFE_WAKE : LIFE_PRIORITY;
    e->address.location=LOW_RESUME; e->address.bank=LOW_BANK;
    e->cadr=06234; e->priority=01000; e->needs_vac=0; e->touch=0;
    if (n==46) e->cadr=010234;
    if (n==47 || n==48) e->cadr=0;
    if (n==49) e->cadr=AGC_NEG_ZERO;
    if (n==50) e->cadr=076321;
    if ((n>=58 && n<65) || n==71) e->priority=037000;
    if (n>=65 && n<=67) e->priority=02000;
    if (n<76) return;
    if(n==80) {
        if(step==0 || step==3) {
            e->operation=LIFE_ADMIT; e->priority=step==0 ? 03000 : 02000;
            e->needs_vac=(uint8_t)(step==0);
            e->address.location=step==0 ? LOW_ENTRY : HIGH_ENTRY;
            e->address.bank=step==0 ? LOW_BANK : HIGH_BANK;
        } else if(step==1 || step==4) e->operation=LIFE_START;
        else if(step==2) { e->operation=LIFE_SLEEP; e->cadr=06011; e->touch=1; }
        else if(step==5) { e->operation=LIFE_WAKE; e->cadr=06011; }
        else if(step==6) {
            e->operation=LIFE_PRIORITY; e->priority=01000; e->touch=2;
            e->address.location=HIGH_RESUME; e->address.bank=HIGH_BANK;
        } else {
            e->operation=LIFE_COMPLETE; e->touch=step==7 ? 3 : 4;
            e->address.bank=step==7 ? LOW_BANK : HIGH_BANK;
        }
        return;
    }
    e->needs_vac=(uint8_t)(n & 1u);
    if (n<78) {
        if (step==0 || step==1) {
            e->operation=LIFE_ADMIT; e->priority=step==0 ? 03000 : 02000;
            e->address.location=step==0 ? LOW_ENTRY : HIGH_ENTRY;
            e->address.bank=step==0 ? LOW_BANK : HIGH_BANK;
        } else if (step==2) e->operation=LIFE_START;
        else if (step==3) { e->operation=LIFE_SLEEP; e->cadr=06011; e->touch=1; }
        else if (step==4) { e->operation=LIFE_WAKE; e->cadr=06011; }
        else if (step==5) {
            e->operation=LIFE_PRIORITY; e->address.location=HIGH_RESUME;
            e->address.bank=HIGH_BANK; e->priority=01000; e->touch=2;
        } else {
            e->operation=LIFE_COMPLETE; e->address.bank=step==6 ? LOW_BANK : HIGH_BANK;
            e->touch=step==6 ? 3 : 4;
        }
    } else {
        if (step==0) { e->operation=LIFE_ADMIT; e->priority=03000; e->address.location=LOW_ENTRY; }
        else if (step==1 || step==4) e->operation=LIFE_START;
        else if (step==2) { e->operation=LIFE_SLEEP; e->cadr=06011; e->touch=1; }
        else if (step==3) { e->operation=LIFE_WAKE; e->cadr=06011; }
        else if (step==5) { e->operation=LIFE_PRIORITY; e->priority=037000;
                           e->address.location=LOW_FINISH; e->touch=2; }
        else { e->operation=LIFE_COMPLETE; e->touch=3; }
    }
}
#ifndef LIFECYCLE_INPUT_ONLY
enum exec_status lifecycle_apply(executive *s,const lifecycle_event *e,exec_address *next) {
    dispatch_touch(s,e->touch);
    next->location=0; next->bank=0;
    if (e->operation==LIFE_SLEEP) return exec_sleep_basic(s,e->cadr,e->address.bank,next);
    if (e->operation==LIFE_WAKE) return exec_wake_basic(s,e->cadr);
    if (e->operation==LIFE_PRIORITY) return exec_priority_basic(s,e->priority,e->address,next);
    if (e->operation==LIFE_ADMIT)
        return exec_admit(s,e->priority,e->address.location,e->address.bank,e->needs_vac);
    if (e->operation==LIFE_START) return exec_start_basic(s,next);
    return exec_complete_basic(s,(agc_word)(e->address.bank & 0160),next);
}
typedef struct {
    exec_native runtime;
    exec_irq_queue queue;
    dispatch_observer observer;
    void *user;
    uint8_t n,step,failed;
} demo;
static void record(demo *d,enum exec_status status,exec_address next) {
    if (d->observer!=0) d->observer(d->user,d->step,d->runtime.state,status,next);
    ++d->step;
    if (status!=EXEC_OK && status!=EXEC_IDLE) d->failed=1;
}
static void payload(executive *s,demo *d,uint8_t pattern) {
    uint8_t i;
    agc_word base=(agc_word)(s->core[0][EXEC_PRIORITY] & EXEC_LOW9);
    for(i=0;i<7;++i) if(s->core[0][i]!=(agc_word)(pattern*0100u+i)) d->failed=1;
    if(s->ovfind!=(agc_word)(pattern & 1u) ||
       s->core[0][EXEC_MODE]!=((pattern & 1u) ? AGC_NEG_ZERO : 0)) d->failed=1;
    for(i=0;i<5;++i) if(base==(agc_word)(EXEC_VAC1USE+i*EXEC_VAC_WORDS+1) &&
        (s->vac[i][0]!=0 || s->vac[i][1]!=(agc_word)(pattern*01000u+7u))) d->failed=1;
}
static enum exec_action low_entry(executive *s,void *user,exec_address *resume) {
    (void)user; dispatch_touch(s,1);
    resume->location=LOW_RESUME; resume->bank=LOW_BANK; return EXEC_SLEEP;
}
static enum exec_action high_entry(executive *s,void *user,exec_address *resume) {
    demo *d=(demo *)user;
    exec_address zero={0,0};
    record(d,exec_native_wake(&d->runtime,06011),zero);
    if(exec_native_request_priority(&d->runtime,01000)!=EXEC_OK) d->failed=1;
    if((s->core[0][EXEC_PRIORITY] & 037000)!=02000) d->failed=1;
    dispatch_touch(s,2);
    resume->location=HIGH_RESUME; resume->bank=HIGH_BANK; return EXEC_REPRIORITIZE;
}
static enum exec_action low_resume(executive *s,void *user,exec_address *resume) {
    demo *d=(demo *)user;
    payload(s,d,1);
    if(d->n<78 || d->n==80) { dispatch_touch(s,3); return EXEC_FINISH; }
    if(exec_native_request_priority(&d->runtime,037000)!=EXEC_OK) d->failed=1;
    dispatch_touch(s,2); resume->location=LOW_FINISH; resume->bank=LOW_BANK;
    return EXEC_REPRIORITIZE;
}
static enum exec_action low_finish(executive *s,void *user,exec_address *resume) {
    (void)resume; payload(s,(demo *)user,2); dispatch_touch(s,3); return EXEC_FINISH;
}
static enum exec_action high_finish(executive *s,void *user,exec_address *resume) {
    (void)resume; payload(s,(demo *)user,2); dispatch_touch(s,4); return EXEC_FINISH;
}
static const exec_mapping jobs[]={
    {{LOW_ENTRY,LOW_BANK},low_entry},{{LOW_RESUME,LOW_BANK},low_resume},
    {{LOW_FINISH,LOW_BANK},low_finish},{{HIGH_ENTRY,HIGH_BANK},high_entry},
    {{HIGH_RESUME,HIGH_BANK},high_finish}
};
int lifecycle_demo(executive *s,uint8_t n,dispatch_observer observer,void *user,
                   const exec_exclusion *exclusion) {
    demo d;
    uint8_t i;
    enum exec_status status=EXEC_OK;
    exec_address zero={0,0};
    lifecycle_prepare(s,n);
    d.n=n; d.step=0; d.failed=0; d.observer=observer; d.user=user;
    if(exec_native_init(&d.runtime,s,jobs,5,&d)!=EXEC_OK) return 0;
    if(exclusion!=0) {
        exec_irq_init(&d.queue);
        if(exec_native_bind_irq(&d.runtime,&d.queue,exclusion)!=EXEC_OK) return 0;
    }
    for(i=0;i<(n<78 ? 2 : 1);++i) {
        lifecycle_event e;
        lifecycle_event_prepare(&e,n,i);
        record(&d,exec_native_admit(&d.runtime,e.priority,e.address,e.needs_vac),zero);
    }
    status=exec_native_start(&d.runtime); record(&d,status,d.runtime.current);
    while(d.runtime.running!=0 && d.step<9) {
        status=exec_native_step(&d.runtime); record(&d,status,d.runtime.current);
    }
    if(n>=78) {
        int vac=(n & 1u)!=0 || n==80;
        if(status!=EXEC_IDLE || s->core[0][EXEC_PRIORITY]!=invert(vac ? 03401 : 03110) ||
           s->ovfind!=1 || (vac && s->vac[0][0]!=0)) d.failed=1;
        if(n==80) record(&d,exec_native_admit(&d.runtime,02000,(exec_address){HIGH_ENTRY,HIGH_BANK},0),zero);
        else record(&d,exec_native_wake(&d.runtime,06011),zero);
        status=exec_native_start(&d.runtime); record(&d,status,d.runtime.current);
        while(d.runtime.running!=0 && d.step<9) {
            status=exec_native_step(&d.runtime); record(&d,status,d.runtime.current);
        }
    }
    return d.failed==0 && d.step==lifecycle_steps(n) && status==EXEC_IDLE;
}
#endif
