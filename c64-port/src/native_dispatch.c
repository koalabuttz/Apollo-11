/* SPDX-License-Identifier: MIT */
#include "native_dispatch.h"
static exec_continuation lookup(const exec_native *r, exec_address a) {
    uint8_t i;
    for (i=0; i<r->count; ++i)
        if (r->mapping[i].address.location == a.location &&
            r->mapping[i].address.bank == a.bank) return r->mapping[i].call;
    return 0;
}
static enum exec_status settle(exec_native *r, enum exec_status status) {
    if (status == EXEC_IDLE) { r->running=0; return status; }
    if (status == EXEC_OK && lookup(r,r->current) == 0) status=EXEC_UNMAPPED;
    if (status != EXEC_OK) { r->fault=status; r->running=0; }
    return status;
}
enum exec_status exec_native_init(exec_native *r, executive *s,
                                 const exec_mapping *m, uint8_t count, void *user) {
    uint8_t i,j;
    if (r == 0 || s == 0 || m == 0 || count == 0) return EXEC_OUT_OF_SCOPE;
    r->state=s; r->mapping=m; r->count=count; r->user=user;
    r->current.location=0; r->current.bank=0; r->running=0; r->fault=EXEC_OK;
    for (i=0; i<count; ++i) {
        if (m[i].call == 0 || m[i].address.location == 0 || m[i].address.location > 07777 ||
            m[i].address.bank > AGC_MASK) return settle(r,EXEC_OUT_OF_SCOPE);
        for (j=0; j<i; ++j)
            if (m[i].address.location == m[j].address.location &&
                m[i].address.bank == m[j].address.bank)
                return settle(r,EXEC_OUT_OF_SCOPE);
    }
    return EXEC_OK;
}
enum exec_status exec_native_start(exec_native *r) {
    enum exec_status status;
    if (r->fault != EXEC_OK) return r->fault;
    if (r->running != 0) return EXEC_OUT_OF_SCOPE;
    status=exec_start_basic(r->state,&r->current);
    if (status == EXEC_OK) r->running=1;
    return settle(r,status);
}
enum exec_status exec_native_step(exec_native *r) {
    exec_address resume={0,0};
    exec_continuation call;
    enum exec_action action;
    enum exec_status status;
    if (r->fault != EXEC_OK) return r->fault;
    if (r->running == 0) return EXEC_IDLE;
    call=lookup(r,r->current);
    if (call == 0) return settle(r,EXEC_UNMAPPED);
    action=call(r->state,r->user,&resume);
    if (action == EXEC_FINISH)
        status=exec_complete_basic(r->state,(agc_word)(r->current.bank & 0160),&r->current);
    else if (action == EXEC_YIELD) {
        if (resume.location == 0 || resume.location > 07777 || resume.bank > AGC_MASK)
            return settle(r,EXEC_OUT_OF_SCOPE);
        if (r->state->newjob == 0) { r->current=resume; status=EXEC_OK; }
        else status=exec_change_basic(r->state,resume,&r->current);
    } else status=EXEC_INVALID_ACTION;
    return settle(r,status);
}
