/* SPDX-License-Identifier: MIT */
#include "waitlist_check.h"
#include "waitlist_vectors.h"
typedef struct { unsigned errors; uint16_t rows; } checks;
static void observe(void *user,uint16_t n,uint8_t step,const waitlist *s,enum wl_status status,exec_address next) {
    checks *c=user;
    uint16_t index=n<156 ? n : n<166 ? (uint16_t)(156+(n-156)*12+step) : (uint16_t)(276+n-166);
    if(wait_digest(s,status,next)!=waitlist_reference_digest[index]) ++c->errors;
    ++c->rows;
}
unsigned waitlist_check(waitlist *s,const exec_exclusion *exclusion) {
    uint16_t n;
    checks c={0,0};
    for(n=0;n<WAIT_CASE_COUNT;++n) {
        if(n>=156 && n<166) c.errors+=wait_native_run(s,n,exclusion,observe,&c);
        else {
            uint8_t token=0;
            wait_event e; exec_address next={0,0}; enum wl_status status;
            wait_prepare(s,n); wait_event_prepare(&e,n,0);
            if(exclusion!=0) token=exclusion->enter(exclusion->user);
            status=wait_apply(s,&e,&next);
            if(exclusion!=0) exclusion->leave(exclusion->user,token);
            observe(&c,n,0,s,status,next);
        }
    }
    if(c.rows!=WAIT_CHECKPOINT_COUNT) ++c.errors;
    return c.errors;
}
