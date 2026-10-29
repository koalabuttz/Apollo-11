/* SPDX-License-Identifier: MIT */
#include "lifecycle_check.h"
#include "lifecycle_cases.h"
#include "lifecycle_vectors.h"
#include "state_digest.h"
typedef struct { uint8_t n,seen; unsigned failures; } checker;
static void check(void *user,uint8_t step,const executive *s,enum exec_status status,exec_address next) {
    checker *c=(checker *)user;
    ++c->seen;
    if(dispatch_state_digest(s,(int)status,next)!=
       lifecycle_reference_digest[lifecycle_vector_index(c->n,step)]) ++c->failures;
}
unsigned lifecycle_check(executive *s,const exec_exclusion *x) {
    checker c={0,0,0};
    for(c.n=0;c.n<LIFECYCLE_CASE_COUNT;++c.n) {
        if(c.n>=76) {
            if(!lifecycle_demo(s,c.n,check,&c,x)) ++c.failures;
        } else {
            uint8_t token=0;
            lifecycle_event e;
            exec_address next={0,0};
            enum exec_status status;
            lifecycle_prepare(s,c.n); lifecycle_event_prepare(&e,c.n,0);
            if(x!=0) token=x->enter(x->user);
            status=lifecycle_apply(s,&e,&next);
            if(x!=0) x->leave(x->user,token);
            check(&c,0,s,status,next);
        }
    }
    if(c.seen!=LIFECYCLE_CHECKPOINT_COUNT) ++c.failures;
    return c.failures;
}
