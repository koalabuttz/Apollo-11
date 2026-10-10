/* SPDX-License-Identifier: MIT */
#include "restart_check.h"
#include "restart_vectors.h"
unsigned rs_check(restart_case *c,const exec_exclusion *x) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=0;n<RS_CASE_COUNT;++n) {
        uint8_t step; rs_prepare(c,n);
        for(step=0;step<rs_steps(n);++step) {
            int status; uint8_t token=0;
            if(x!=0) token=x->enter(x->user);
            status=rs_apply(c,n,step);
            if(x!=0) x->leave(x->user,token);
            if(rs_digest(c,status)!=restart_reference_digest[row]) ++failures;
            ++row;
        }
    }
    if(row!=226) ++failures;
    return failures;
}
typedef struct { unsigned failures; uint8_t seen,count; exec_native *runtime; } demo_trace;
static enum exec_action recovered_job(executive *e,void *user,exec_address *resume) {
    demo_trace *trace=user;
    uint8_t group=(uint8_t)(trace->runtime->current.location-03000);
    (void)resume; (void)e;
    if(group>=6 || (trace->seen & (uint8_t)(1u << group))!=0) ++trace->failures;
    else trace->seen=(uint8_t)(trace->seen | (uint8_t)(1u << group));
    ++trace->count;
    return EXEC_FINISH;
}
unsigned rs_demo(restart_case *c,const exec_exclusion *x) {
    exec_native runtime; exec_mapping mappings[6];
    demo_trace trace={0,0,0,&runtime}; uint8_t i,token=0;
    enum rs_status status;
    exec_irq_queue queue;
    rs_prepare(c,186);
    for(i=0;i<6;++i) { mappings[i].address=c->phases.address[i]; mappings[i].call=recovered_job; }
    if(exec_native_init(&runtime,&c->exec,mappings,6,&trace)!=EXEC_OK) ++trace.failures;
    exec_irq_init(&queue);
    if(x!=0 && exec_native_bind_irq(&runtime,&queue,x)!=EXEC_OK) ++trace.failures;
    {
        exec_address failed={03200,06005};
        if(exec_native_admit(&runtime,03000,failed,1)!=EXEC_NO_CORE || c->exec.vac[0][0]!=0) ++trace.failures;
        if(exec_native_step(&runtime)!=EXEC_NO_CORE || trace.count!=0) ++trace.failures;
    }
    if(x!=0) token=x->enter(x->user);
    status=rs_recover(&c->record,&c->exec,&c->wait,&c->phases,c->caller);
    if(x!=0) x->leave(x->user,token);
    if(status!=RS_OK || c->record.failreg[0]!=01202 || c->exec.alarm!=0 ||
       c->exec.vac[0][0]!=0 || c->wait.task[0].location!=WL_END_LOCATION) ++trace.failures;
    /* Cleaning raw scheduler state must never revive a stale C runtime. */
    if(exec_native_step(&runtime)!=EXEC_NO_CORE || trace.count!=0) ++trace.failures;
    if(exec_native_init(&runtime,&c->exec,mappings,6,&trace)!=EXEC_OK) ++trace.failures;
    if(x!=0 && exec_native_bind_irq(&runtime,&queue,x)!=EXEC_OK) ++trace.failures;
    for(i=0;i<6;++i) {
        enum exec_status stepped=exec_native_step(&runtime);
        if(stepped!=(i==5 ? EXEC_IDLE : EXEC_OK)) ++trace.failures;
    }
    if(trace.count!=6 || trace.seen!=077 || c->exec.vac[0][0]!=0400 || c->record.failreg[0]!=01202) ++trace.failures;
    return trace.failures;
}
