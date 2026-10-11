/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_check.h"
#include "cm_waitlist_vectors.h"
typedef struct { unsigned errors; uint16_t rows; } checks;
static void observe(void *user,uint16_t n,uint8_t step,const cm_waitlist *s,enum cm_wl_status status,exec_address next) {
    checks *c=user;
    uint16_t index=n<156 ? n : n<166 ? (uint16_t)(156+(n-156)*12+step) : (uint16_t)(276+n-166);
    const uint16_t *v=cm_waitlist_reference[index];
    uint8_t i;
    if(*v++!=(uint16_t)status || *v++!=s->time3) ++c->errors;
    for(i=0;i<8;++i) if(*v++!=s->interval[i]) ++c->errors;
    for(i=0;i<9;++i) { if(*v++!=s->task[i].location) ++c->errors; if(*v++!=s->task[i].bank) ++c->errors; }
    if(*v++!=s->ruptagn || *v++!=s->alarm || *v++!=next.location || *v!=next.bank) ++c->errors;
    ++c->rows;
}
unsigned cm_waitlist_check(cm_waitlist *s,const exec_exclusion *exclusion) {
    uint16_t n;
    checks c={0,0};
    for(n=0;n<CM_WAIT_CASE_COUNT;++n) {
        if(n>=156 && n<166) c.errors+=cm_wait_native_run(s,n,exclusion,observe,&c);
        else {
            uint8_t token=0;
            cm_wait_event e; exec_address next={0,0}; enum cm_wl_status status;
            cm_wait_prepare(s,n); cm_wait_event_prepare(&e,n,0);
            if(exclusion!=0) token=exclusion->enter(exclusion->user);
            status=cm_wait_apply(s,&e,&next);
            if(exclusion!=0) exclusion->leave(exclusion->user,token);
            observe(&c,n,0,s,status,next);
        }
    }
    if(c.rows!=CM_WAIT_CHECKPOINT_COUNT) ++c.errors;
    return c.errors;
}
