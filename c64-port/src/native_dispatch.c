/* SPDX-License-Identifier: MIT */
#include "irq_delivery.h"
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
    r->requests=0; r->exclusion=0;
    r->requested_priority=0; r->in_callback=0;
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
static enum exec_status fail(exec_native *r, enum exec_status status) {
    if (status != EXEC_OK) { r->fault=status; r->running=0; }
    return status;
}
static enum exec_status start_locked(exec_native *r) {
    enum exec_status status;
    if (r->state->newjob == AGC_NEG_ZERO) return EXEC_IDLE;
    status=exec_start_basic(r->state,&r->current);
    if (status == EXEC_OK) r->running=1;
    return settle(r,status);
}
enum exec_status exec_native_start(exec_native *r) {
    enum exec_status status;
    uint8_t token;
    if (r->fault != EXEC_OK) return r->fault;
    if (r->running != 0) return EXEC_OUT_OF_SCOPE;
    token=exec_native_lock(r);
    status=exec_irq_service_locked(r);
    if (status == EXEC_OK) status=start_locked(r);
    else status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_poll(exec_native *r) {
    enum exec_status status;
    uint8_t token;
    if (r->fault != EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    status=fail(r,exec_irq_service_locked(r));
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_admit(exec_native *r, agc_word priority,
                                  exec_address address, int needs_vac) {
    enum exec_status status;
    uint8_t token;
    if (r->fault != EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    status=exec_irq_service_locked(r);
    if (status == EXEC_OK)
        status=exec_admit(r->state,priority,address.location,address.bank,needs_vac);
    status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_step(exec_native *r) {
    exec_address resume={0,0};
    exec_continuation call=0;
    enum exec_action action;
    enum exec_status status;
    uint8_t token;
    if (r->fault != EXEC_OK) return r->fault;
    if (r->in_callback != 0) return EXEC_OUT_OF_SCOPE;
    token=exec_native_lock(r);
    status=exec_irq_service_locked(r);
    if (status == EXEC_OK && r->running == 0) status=start_locked(r);
    if (status == EXEC_OK) {
        call=lookup(r,r->current);
        if (call == 0) status=EXEC_UNMAPPED;
    }
    if (status != EXEC_OK && status != EXEC_IDLE) status=fail(r,status);
    if (status==EXEC_OK) { r->requested_priority=0; r->in_callback=1; }
    exec_native_unlock(r,token);
    if (status != EXEC_OK) return status;
    action=call(r->state,r->user,&resume); /* IRQs follow the caller's state. */
    token=exec_native_lock(r);
    r->in_callback=0;
    /* A callback helper may already have latched a fault. Preserve its exact
     * boundary/status; never complete or exchange that failed running job. */
    status=r->fault != EXEC_OK ? r->fault : exec_irq_service_locked(r);
    if (status==EXEC_OK && r->requested_priority!=0 && action!=EXEC_REPRIORITIZE)
        status=EXEC_INVALID_ACTION;
    if (status == EXEC_OK) {
        if (action == EXEC_FINISH)
            status=exec_complete_basic(r->state,(agc_word)(r->current.bank & 0160),&r->current);
        else if (action == EXEC_YIELD) {
            if (resume.location == 0 || resume.location > 07777 || resume.bank > AGC_MASK)
                status=EXEC_OUT_OF_SCOPE;
            else if (r->state->newjob == 0) { r->current=resume; status=EXEC_OK; }
            else status=exec_change_basic(r->state,resume,&r->current);
        } else if (action==EXEC_SLEEP) {
            if (resume.location<02000 || resume.location>03777 ||
                (resume.bank & (agc_word)~UINT16_C(076167))!=0 ||
                (resume.bank & 0177)!=(r->current.bank & 0177)) status=EXEC_OUT_OF_SCOPE;
            else if (lookup(r,resume)==0) status=EXEC_UNMAPPED;
            else status=exec_sleep_basic(r->state,
                (agc_word)((resume.location-02000) | (resume.bank & 076000)),
                r->current.bank,&r->current);
        } else if (action==EXEC_REPRIORITIZE) {
            if (r->requested_priority==0) status=EXEC_INVALID_ACTION;
            else if (lookup(r,resume)==0) status=EXEC_UNMAPPED;
            else status=exec_priority_basic(r->state,r->requested_priority,resume,&r->current);
        } else status=EXEC_INVALID_ACTION;
    }
    status=settle(r,status);
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_wake(exec_native *r, agc_word cadr) {
    enum exec_status status;
    uint8_t token;
    if (r->fault!=EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    status=exec_irq_service_locked(r);
    if (status==EXEC_OK) {
        status=exec_wake_basic(r->state,cadr);
        if (status==EXEC_OK) {
            uint8_t slot=(uint8_t)(r->state->locctr/EXEC_CORE_WORDS);
            exec_address a={r->state->core[slot][EXEC_LOC],r->state->core[slot][EXEC_BANK]};
            if (lookup(r,a)==0) status=EXEC_UNMAPPED;
        }
    }
    if (status!=EXEC_NOT_FOUND) status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
enum exec_status exec_native_request_priority(exec_native *r, agc_word priority) {
    enum exec_status status=EXEC_OK;
    uint8_t token;
    if (r->fault!=EXEC_OK) return r->fault;
    token=exec_native_lock(r);
    if (r->in_callback==0 || priority==0 || (priority & UINT16_C(0140777))!=0)
        status=EXEC_OUT_OF_SCOPE;
    else r->requested_priority=priority;
    status=fail(r,status);
    exec_native_unlock(r,token);
    return status;
}
