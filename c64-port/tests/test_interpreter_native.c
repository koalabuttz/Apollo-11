/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "interpreter_native_check.h"
static enum exec_action finish(executive *s,void *u,exec_address *a) {
    (void)s; (void)u; (void)a; return EXEC_FINISH;
}
int main(void) {
    unsigned n,step;
    in_run r;
    exec_context_mapping bad[]={{{02000,06000},EXEC_CONTEXT_BASIC,finish},
                                {{02000,06000},EXEC_CONTEXT_BASIC,finish}};
    for(n=0;n<IN_CASE_COUNT;++n) {
        in_run_init(&r,n);
        for(step=0;step<in_steps(n);++step) {
            enum exec_status status=in_run_step(&r,step);
            assert(in_checkpoint(&r,n,step,status));
        }
    }
    in_run_init(&r,0);
    assert(exec_native_context_init(&r.runtime,&r.state,bad,2,&r)==EXEC_OUT_OF_SCOPE);
    bad[1].kind=EXEC_CONTEXT_INTERPRETIVE;
    assert(exec_native_context_init(&r.runtime,&r.state,bad,2,&r)==EXEC_OK);
    assert(exec_native_is_mapped(&r.runtime,bad[1].address,EXEC_CONTEXT_INTERPRETIVE));
    bad[1].kind=(enum exec_context_kind)9;
    assert(exec_native_context_init(&r.runtime,&r.state,bad,2,&r)==EXEC_OUT_OF_SCOPE);
    in_run_init(&r,0);
    for(step=0;step<4;++step) assert(in_run_step(&r,step)==EXEC_OK);
    r.runtime.interpreter_control.edop=1;
    assert(in_run_step(&r,4)==EXEC_OUT_OF_SCOPE);
    n=r.callbacks;
    assert(exec_native_step(&r.runtime)==EXEC_OUT_OF_SCOPE && r.callbacks==n);
    in_run_init(&r,0);
    for(step=0;step<4;++step) assert(in_run_step(&r,step)==EXEC_OK);
    r.runtime.count=7; /* Keep low mappings, remove the already-admitted high target. */
    {
        executive before=r.state;
        in_touch(&before,1); /* The callback payload precedes its return boundary. */
        assert(in_run_step(&r,4)==EXEC_UNMAPPED);
        assert(memcmp(&before,&r.state,sizeof before)==0);
        assert(r.runtime.current.location==IN_LOW_A);
    }
    in_run_init(&r,0);
    r.runtime.requests=0; r.state.alarm=01202;
    assert(exec_native_start(&r.runtime)==EXEC_BAILOUT_PENDING);
    assert(r.callbacks==0 && r.state.newjob==AGC_NEG_ZERO);
    puts("PASS: 140 mixed native/assembly state checkpoints, frame/payload/guard and typed mapping fault checks");
    return 0;
}
