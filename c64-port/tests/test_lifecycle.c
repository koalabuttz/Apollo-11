/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lifecycle_check.h"
#include "lifecycle_cases.h"
typedef struct { uint8_t enabled; unsigned enters,leaves; } guard;
static uint8_t enter(void *user) {
    guard *g=(guard *)user; uint8_t token=g->enabled;
    g->enabled=0; ++g->enters; return token;
}
static void leave(void *user,uint8_t token) {
    guard *g=(guard *)user; assert(g->enabled==0);
    g->enabled=token; ++g->leaves;
}
static uint8_t behavior;
static enum exec_action bad(executive *s,void *user,exec_address *resume) {
    exec_native *r=(exec_native *)user;
    (void)s;
    resume->location=LOW_RESUME; resume->bank=LOW_BANK;
    if(behavior==0) { resume->location=LOW_FINISH; return EXEC_SLEEP; }
    if(behavior==1) { resume->bank=06023; return EXEC_SLEEP; }
    if(behavior==2) return EXEC_REPRIORITIZE; /* No staged priority. */
    if(behavior==3) {
        assert(exec_native_request_priority(r,0)==EXEC_OUT_OF_SCOPE);
        return EXEC_FINISH;
    }
    if(behavior==5) { resume->bank=(agc_word)(LOW_BANK | 0400); return EXEC_SLEEP; }
    assert(exec_native_step(r)==EXEC_OUT_OF_SCOPE); /* Re-entry is rejected. */
    assert(exec_native_request_priority(r,01000)==EXEC_OK);
    return EXEC_YIELD; /* Staged priority must have a matching returned action. */
}
static const exec_mapping map[]={{{LOW_ENTRY,LOW_BANK},bad},{{LOW_RESUME,LOW_BANK},bad}};
static void setup(exec_native *r,executive *s,exec_irq_queue *q,exec_exclusion *x) {
    exec_init(s); exec_irq_init(q);
    assert(exec_native_init(r,s,map,2,r)==EXEC_OK);
    assert(exec_native_bind_irq(r,q,x)==EXEC_OK);
}
int main(void) {
    executive s,before;
    exec_native runtime;
    exec_irq_queue queue;
    exec_address resume={LOW_RESUME,LOW_BANK},next={01234,05670},old_next=next;
    enum exec_status expected[]={EXEC_UNMAPPED,EXEC_OUT_OF_SCOPE,EXEC_INVALID_ACTION,
                                 EXEC_OUT_OF_SCOPE,EXEC_INVALID_ACTION,EXEC_OUT_OF_SCOPE};
    guard g={1,0,0}; exec_exclusion x={enter,leave,&g};
    uint8_t i;
    assert(lifecycle_check(&s,0)==0);
    assert(lifecycle_check(&s,&x)==0 && g.enabled==1 && g.enters==g.leaves);
    g.enabled=0;
    assert(lifecycle_check(&s,&x)==0 && g.enabled==0 && g.enters==g.leaves);
    g.enabled=1;
    lifecycle_prepare(&s,0); before=s;
    assert(exec_sleep_basic(&s,06011,010,&next)==EXEC_OUT_OF_SCOPE);
    assert(exec_sleep_basic(&s,0100000,LOW_BANK,&next)==EXEC_OUT_OF_SCOPE);
    assert(exec_sleep_basic(&s,06011,LOW_BANK,0)==EXEC_OUT_OF_SCOPE);
    assert(exec_priority_basic(&s,0,resume,&next)==EXEC_OUT_OF_SCOPE);
    resume.location=0; assert(exec_priority_basic(&s,01000,resume,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&s,&before,sizeof s)==0 && memcmp(&next,&old_next,sizeof next)==0);
    resume.location=LOW_RESUME;
    s.core[1][EXEC_LOC]=(agc_word)(02000 ^ AGC_MASK); before=s;
    assert(exec_sleep_basic(&s,06011,LOW_BANK,&next)==EXEC_OUT_OF_SCOPE);
    assert(exec_priority_basic(&s,01000,resume,&next)==EXEC_OUT_OF_SCOPE);
    assert(memcmp(&s,&before,sizeof s)==0);
    lifecycle_prepare(&s,22); s.newjob=0; before=s;
    assert(exec_wake_basic(&s,06234)==EXEC_OUT_OF_SCOPE && memcmp(&s,&before,sizeof s)==0);
    s.newjob=13; before=s;
    assert(exec_wake_basic(&s,06234)==EXEC_OUT_OF_SCOPE && memcmp(&s,&before,sizeof s)==0);
    s.alarm=01202; before=s;
    assert(exec_wake_basic(&s,06234)==EXEC_BAILOUT_PENDING);
    assert(exec_sleep_basic(&s,06011,LOW_BANK,&next)==EXEC_BAILOUT_PENDING);
    assert(exec_priority_basic(&s,01000,resume,&next)==EXEC_BAILOUT_PENDING);
    assert(memcmp(&s,&before,sizeof s)==0);
    for(i=0;i<6;++i) {
        behavior=i; setup(&runtime,&s,&queue,&x);
        assert(exec_native_admit(&runtime,03000,(exec_address){LOW_ENTRY,LOW_BANK},1)==EXEC_OK);
        assert(exec_native_start(&runtime)==EXEC_OK); before=s;
        assert(exec_native_step(&runtime)==expected[i] && runtime.fault==expected[i]);
        assert(memcmp(&s,&before,sizeof s)==0 && g.enabled==1 && runtime.in_callback==0);
        assert(exec_native_step(&runtime)==expected[i]);
    }
    setup(&runtime,&s,&queue,&x);
    assert(exec_native_wake(&runtime,06011)==EXEC_NOT_FOUND && runtime.fault==EXEC_OK);
    assert(s.locctr==077776 && exec_native_step(&runtime)==EXEC_IDLE);
    assert(exec_native_request_priority(&runtime,01000)==EXEC_OUT_OF_SCOPE);
    /* Wake can commit original state before detecting an unsupported mapping;
     * its documented failure is fail-stop, not rollback. */
    setup(&runtime,&s,&queue,&x);
    s.core[0][EXEC_PRIORITY]=(agc_word)(03110 ^ AGC_MASK);
    s.core[0][EXEC_LOC]=06012; s.core[0][EXEC_BANK]=022;
    assert(exec_native_wake(&runtime,06012)==EXEC_UNMAPPED && s.core[0][EXEC_PRIORITY]==03110);
    assert(runtime.fault==EXEC_UNMAPPED && g.enters==g.leaves);
    puts("PASS: 81 lifecycle cases / 115 assembly-derived checkpoints, 5 native sequences, guarded/masked delivery and lifecycle fault boundaries");
    return 0;
}
