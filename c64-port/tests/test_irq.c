/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "irq_delivery.h"
#include "dispatch_cases.h"
#include "scenarios.h"
typedef struct { uint8_t enabled; unsigned entered,left; } guard;
static uint8_t enter(void *user) {
    guard *g=(guard *)user;
    uint8_t old=g->enabled;
    g->enabled=0; ++g->entered; return old;
}
static void leave(void *user,uint8_t old) {
    guard *g=(guard *)user;
    assert(g->enabled==0); g->enabled=old; ++g->left;
}
typedef struct { exec_irq_queue *queue; guard *g; unsigned trace; } jobs;
static enum exec_action low(executive *s,void *user,exec_address *resume) {
    jobs *j=(jobs *)user;
    executive before=*s;
    exec_request request={03000,{HIGH_ENTRY,HIGH_BANK},1};
    uint8_t old;
    assert(j->g->enabled==1);
    j->trace=j->trace*10u+1u;
    old=enter(j->g); /* Simulate IRQ entry, not a host thread. */
    assert(exec_irq_post(j->queue,&request)==EXEC_OK);
    leave(j->g,old);
    assert(memcmp(&before,s,sizeof before)==0); /* IRQ cannot preempt/admit. */
    s->core[0][EXEC_MPAC]=01234; s->ovfind=1;
    resume->location=LOW_RESUME; resume->bank=LOW_BANK; return EXEC_YIELD;
}
static enum exec_action high(executive *s,void *user,exec_address *resume) {
    jobs *j=(jobs *)user;
    (void)s; (void)resume;
    assert(j->g->enabled==1); j->trace=j->trace*10u+2u; return EXEC_FINISH;
}
static enum exec_action resumed(executive *s,void *user,exec_address *resume) {
    jobs *j=(jobs *)user;
    (void)resume;
    assert(j->g->enabled==1); assert(s->core[0][EXEC_MPAC]==01234 && s->ovfind==1);
    j->trace=j->trace*10u+3u; return EXEC_FINISH;
}
static const exec_mapping map[]={{{LOW_ENTRY,LOW_BANK},low},
    {{LOW_RESUME,LOW_BANK},resumed},{{HIGH_ENTRY,HIGH_BANK},high}};
static enum exec_action failing(executive *s,void *user,exec_address *resume) {
    exec_native *r=(exec_native *)user;
    exec_address address={LOW_ENTRY,LOW_BANK};
    (void)s; (void)resume;
    assert(exec_native_admit(r,03000,address,1)==EXEC_NO_CORE);
    return EXEC_FINISH; /* Must not release resources after the helper fault. */
}
static void setup(exec_native *r,executive *s,exec_irq_queue *q,exec_exclusion *x,jobs *j) {
    exec_init(s); exec_irq_init(q);
    assert(exec_native_init(r,s,map,3,j)==EXEC_OK);
    assert(exec_native_bind_irq(r,q,x)==EXEC_OK);
}
int main(void) {
    exec_native r;
    executive s,expected,before;
    exec_irq_queue q;
    guard g={1,0,0};
    exec_exclusion x={enter,leave,&g};
    jobs j={&q,&g,0};
    exec_request a={02000,{LOW_ENTRY,LOW_BANK},0};
    uint16_t n;
    uint8_t token,inner;
    setup(&r,&s,&q,&x,&j);
    token=exec_native_lock(&r); inner=exec_native_lock(&r);
    assert(g.enabled==0); exec_native_unlock(&r,inner); assert(g.enabled==0);
    exec_native_unlock(&r,token); assert(g.enabled==1);
    assert(exec_native_admit(&r,a.priority,a.address,0)==EXEC_OK);
    assert(exec_native_start(&r)==EXEC_OK);
    assert(exec_native_bind_irq(&r,&q,&x)==EXEC_OUT_OF_SCOPE);
    assert(exec_native_step(&r)==EXEC_OK && r.current.location==HIGH_ENTRY);
    assert(j.trace==1 && q.write==q.read); /* Admission only on callback return. */
    assert(exec_native_step(&r)==EXEC_OK && r.current.location==LOW_RESUME);
    assert(exec_native_step(&r)==EXEC_IDLE && j.trace==123);
    assert(exec_native_step(&r)==EXEC_IDLE);
    assert(g.enabled==1 && g.entered==g.left);
    /* Idle request delivery starts the native job at the next step. */
    a.address.location=HIGH_ENTRY; a.address.bank=HIGH_BANK;
    assert(exec_irq_post(&q,&a)==EXEC_OK);
    assert(exec_native_step(&r)==EXEC_IDLE && j.trace==1232);
    /* FIFO, whole-payload publication and ticket wrap. Exact same state as
     * ordinary core admission, backed by original-assembly differential tests. */
    for(n=0;n<600;++n) {
        setup(&r,&s,&q,&x,&j); q.write=(uint8_t)n; q.read=(uint8_t)n;
        expected=s;
        a.priority=02000; a.address.location=LOW_ENTRY; a.address.bank=LOW_BANK;
        a.needs_vac=0; assert(exec_irq_post(&q,&a)==EXEC_OK);
        assert(exec_admit(&expected,a.priority,a.address.location,a.address.bank,0)==EXEC_OK);
        a.priority=03000; a.address.location=HIGH_ENTRY; a.address.bank=HIGH_BANK;
        a.needs_vac=1; assert(exec_irq_post(&q,&a)==EXEC_OK);
        assert(exec_admit(&expected,a.priority,a.address.location,a.address.bank,1)==EXEC_OK);
        assert(exec_native_poll(&r)==EXEC_OK && q.read==q.write);
        assert(memcmp(&expected,&s,sizeof s)==0);
    }
    /* Overflow is a distinct native fail-stop; accepted payloads stay buffered. */
    setup(&r,&s,&q,&x,&j); before=s;
    for(n=0;n<4;++n) assert(exec_irq_post(&q,&a)==EXEC_OK);
    assert(exec_irq_post(&q,&a)==EXEC_QUEUE_FULL && q.write==4 && q.read==0);
    assert(exec_native_poll(&r)==EXEC_QUEUE_FULL && r.fault==EXEC_QUEUE_FULL);
    assert(memcmp(&before,&s,sizeof s)==0 && q.read==0);
    for(n=0;n<300;++n) assert(exec_irq_post(&q,&a)==EXEC_QUEUE_FULL);
    assert(q.rejected==255 && q.write==4 && q.read==0);
    assert(exec_native_step(&r)==EXEC_QUEUE_FULL);
    /* Each resource failure preserves the original boundary, including VAC
     * reservation before a full-core 1202, and leaves the failed request queued. */
    setup(&r,&s,&q,&x,&j);
    { scenario fixture; scenario_prepare(&fixture,25); s=fixture.state; }
    expected=s;
    assert(exec_admit(&expected,a.priority,a.address.location,a.address.bank,1)==EXEC_NO_CORE);
    assert(exec_irq_post(&q,&a)==EXEC_OK);
    assert(exec_native_poll(&r)==EXEC_NO_CORE && s.alarm==01202 && q.read==0);
    assert(memcmp(&s,&expected,sizeof s)==0 && s.vac[0][0]==0);
    assert(exec_native_poll(&r)==EXEC_NO_CORE); /* No second admission after fault. */
    /* Failure inside the callback retains 1202 and its reserved VAC, even if
     * the callback returns FINISH. A later bailout precheck cannot replace it. */
    {
        const exec_mapping failure_map={ {LOW_ENTRY,LOW_BANK},failing };
        scenario fixture;
        scenario_prepare(&fixture,25); s=fixture.state; expected=s;
        assert(exec_admit(&expected,03000,LOW_ENTRY,LOW_BANK,1)==EXEC_NO_CORE);
        exec_irq_init(&q);
        assert(exec_native_init(&r,&s,&failure_map,1,&r)==EXEC_OK);
        assert(exec_native_bind_irq(&r,&q,&x)==EXEC_OK);
        r.current=failure_map.address; r.running=1;
        assert(exec_native_step(&r)==EXEC_NO_CORE && r.fault==EXEC_NO_CORE);
        assert(memcmp(&s,&expected,sizeof s)==0 && g.enabled==1);
    }
    setup(&r,&s,&q,&x,&j);
    for(n=0;n<5;++n) s.vac[n][0]=0;
    assert(exec_irq_post(&q,&a)==EXEC_OK);
    assert(exec_native_poll(&r)==EXEC_NO_VAC && s.alarm==01201 && q.read==0);
    setup(&r,&s,&q,&x,&j); s.alarm=01202; before=s;
    assert(exec_irq_post(&q,&a)==EXEC_OK);
    assert(exec_native_poll(&r)==EXEC_BAILOUT_PENDING && q.read==0);
    assert(memcmp(&s,&before,sizeof s)==0);
    setup(&r,&s,&q,&x,&j); a.address.location=02333; before=s;
    assert(exec_irq_post(&q,&a)==EXEC_OK);
    assert(exec_native_poll(&r)==EXEC_UNMAPPED && q.read==0);
    assert(memcmp(&s,&before,sizeof s)==0);
    setup(&r,&s,&q,&x,&j); a.priority=0;
    assert(exec_irq_post(&q,&a)==EXEC_OUT_OF_SCOPE && q.write==0);
    /* Preserve an already-disabled caller on successful and failing paths. */
    g.enabled=0; a.priority=02000; a.address.location=HIGH_ENTRY;
    assert(exec_native_admit(&r,a.priority,a.address,0)==EXEC_OK && g.enabled==0);
    assert(exec_native_start(&r)==EXEC_OK && g.enabled==0);
    a.priority=0;
    assert(exec_native_admit(&r,a.priority,a.address,0)==EXEC_OUT_OF_SCOPE && g.enabled==0);
    assert(g.entered==g.left);
    puts("PASS: IRQ safe points, nested exclusion, idle startup, FIFO/wrap (600 cases), overflow, bailout and unmapped-request faults");
    return 0;
}
