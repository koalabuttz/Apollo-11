/* SPDX-License-Identifier: MIT */
#include "dispatch_check.h"
#include "dispatch_cases.h"
#include "state_digest.h"
#include "reference_vectors.h"
typedef struct { uint8_t n,seen; unsigned failures; } checker;
static void check(void *user,uint8_t step,const executive *s,enum exec_status status,exec_address next) {
    checker *c=(checker *)user;
    ++c->seen;
    if (dispatch_state_digest(s,(int)status,next)!=
        dispatch_reference_digest[dispatch_vector_index(c->n,step)]) ++c->failures;
}
unsigned dispatch_check(executive *s) {
    checker c;
    uint8_t step;
    c.failures=0; c.seen=0;
    for (c.n=0;c.n<DISPATCH_CASE_COUNT;++c.n) {
        if (c.n>=DISPATCH_SEQUENCE_FIRST && c.n<50) {
            if (!dispatch_demo(s,c.n,check,&c)) ++c.failures;
        } else {
            dispatch_prepare(s,c.n);
            for (step=0;step<dispatch_steps(c.n);++step) {
                dispatch_event e;
                exec_address next={0,0};
                enum exec_status status;
                dispatch_event_prepare(&e,c.n,step);
                status=dispatch_apply(s,&e,&next);
                check(&c,step,s,status,next);
            }
        }
    }
    if (c.seen!=109) ++c.failures;
    return c.failures;
}
