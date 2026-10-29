/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_native.h"
static uint8_t lock(cm_wl_native *r) { return r->exclusion!=0 ? r->exclusion->enter(r->exclusion->user) : 0; }
static void unlock(cm_wl_native *r,uint8_t token) { if(r->exclusion!=0) r->exclusion->leave(r->exclusion->user,token); }
static int valid(exec_address a) {
    return a.location!=0 && a.location<=07777 && (a.bank & (agc_word)~UINT16_C(076167))==0;
}
static int dummy(exec_address a) { return a.location==CM_WL_END_LOCATION && a.bank==CM_WL_END_BANK; }
static cm_wl_task_call lookup(cm_wl_native *r,exec_address a) {
    uint8_t i;
    for(i=0;i<r->count;++i) if(r->mapping[i].address.location==a.location && r->mapping[i].address.bank==a.bank) return r->mapping[i].call;
    return 0;
}
static enum cm_wl_status fault(cm_wl_native *r,enum cm_wl_status status) {
    if(status!=CM_WL_OK && status!=CM_WL_IDLE) { r->fault=status; r->running=0; }
    return status;
}
static enum cm_wl_status start_locked(cm_wl_native *r) {
    enum cm_wl_status status;
    if(r->state->alarm!=0) return CM_WL_ALARM_PENDING;
    if(r->pending==0) return CM_WL_IDLE;
    if(!dummy(r->state->task[0]) && lookup(r,r->state->task[0])==0) return CM_WL_UNMAPPED;
    status=cm_wl_begin(r->state,&r->current);
    if(status==CM_WL_OK) { r->pending=0; r->running=1; }
    return status;
}
enum cm_wl_status cm_wl_native_init(cm_wl_native *r,cm_waitlist *s,const cm_wl_mapping *m,uint8_t count,void *user,const exec_exclusion *exclusion) {
    uint8_t i,j;
    if(r==0 || s==0 || m==0 || count==0 || (exclusion!=0 && (exclusion->enter==0 || exclusion->leave==0))) return CM_WL_OUT_OF_SCOPE;
    r->state=s; r->mapping=m; r->count=count; r->user=user; r->exclusion=exclusion;
    r->pending=0; r->running=0; r->in_callback=0; r->fault=CM_WL_OK; r->current.location=0; r->current.bank=0;
    for(i=0;i<count;++i) {
        if(!valid(m[i].address) || m[i].address.location==CM_WL_END_LOCATION || m[i].call==0) return fault(r,CM_WL_OUT_OF_SCOPE);
        for(j=0;j<i;++j) if(m[i].address.location==m[j].address.location && m[i].address.bank==m[j].address.bank) return fault(r,CM_WL_OUT_OF_SCOPE);
    }
    return CM_WL_OK;
}
enum cm_wl_status cm_wl_native_schedule(cm_wl_native *r,agc_word delay,exec_address task) {
    enum cm_wl_status status;
    uint8_t token;
    if(r->fault!=CM_WL_OK) return r->fault;
    token=lock(r);
    status=!valid(task) ? CM_WL_OUT_OF_SCOPE : lookup(r,task)==0 ? CM_WL_UNMAPPED : cm_wl_schedule(r->state,delay,task);
    status=fault(r,status); unlock(r,token); return status;
}
enum cm_wl_status cm_wl_native_advance(cm_wl_native *r,uint16_t ticks) {
    enum cm_wl_status status=CM_WL_OK;
    int due;
    uint8_t token;
    if(r->fault!=CM_WL_OK) return r->fault;
    if(r->in_callback!=0) return CM_WL_REENTER;
    token=lock(r);
    /* WAIT2 can distinguish a pending overflow only within its 0200 window.
     * Refuse ambiguous elapsed time rather than silently misorder new tasks. */
    if(r->state->time3<=037777 && ((r->pending!=0 &&
       (uint32_t)r->state->time3+ticks>=128u) || (r->pending==0 &&
       (uint32_t)ticks>=(uint32_t)(16384u-r->state->time3)+128u))) {
        status=fault(r,CM_WL_SOURCE_OVERFLOW); unlock(r,token); return status;
    }
    due=cm_wl_advance(r->state,ticks);
    if(due<0) status=r->state->alarm!=0 ? CM_WL_ALARM_PENDING : CM_WL_OUT_OF_SCOPE;
    else if(due!=0) r->pending=1;
    status=fault(r,status); unlock(r,token); return status;
}
enum cm_wl_status cm_wl_native_start(cm_wl_native *r) {
    enum cm_wl_status status;
    uint8_t token;
    if(r->fault!=CM_WL_OK) return r->fault;
    if(r->in_callback!=0 || r->running!=0) return CM_WL_REENTER;
    token=lock(r); status=fault(r,start_locked(r)); unlock(r,token); return status;
}
enum cm_wl_status cm_wl_native_step(cm_wl_native *r) {
    enum cm_wl_status status;
    uint8_t token;
    cm_wl_task_call call;
    if(r->fault!=CM_WL_OK) return r->fault;
    if(r->in_callback!=0) return CM_WL_REENTER;
    token=lock(r);
    status=r->state->alarm!=0 ? CM_WL_ALARM_PENDING : r->running!=0 ? CM_WL_OK : start_locked(r);
    if(status==CM_WL_OK) {
        call=lookup(r,r->current);
        if(!dummy(r->current) && call==0) status=CM_WL_UNMAPPED;
        else {
            r->in_callback=1;
            status=call!=0 ? call(r,r->user) : CM_WL_OK;
            r->in_callback=0;
            if(r->fault!=CM_WL_OK) status=r->fault;
            if(status==CM_WL_OK && r->state->ruptagn==1 && !dummy(r->state->task[0]) && lookup(r,r->state->task[0])==0) status=CM_WL_UNMAPPED;
            if(status==CM_WL_OK) {
                status=cm_wl_taskover(r->state,&r->current); r->running=(uint8_t)(status==CM_WL_OK);
            } else if(status==CM_WL_IDLE) status=CM_WL_CALLBACK_FAULT;
        }
    }
    status=fault(r,status); unlock(r,token); return status;
}

void cm_wl_clock_init(cm_wl_clock *clock) { clock->ticks=0; clock->overflow=0; }
enum cm_wl_status cm_wl_clock_post(cm_wl_clock *clock) {
    if(clock->overflow!=0 || clock->ticks==UINT16_MAX) { clock->overflow=1; return CM_WL_SOURCE_OVERFLOW; }
    ++clock->ticks; return CM_WL_OK;
}
enum cm_wl_status cm_wl_native_collect(cm_wl_native *r,cm_wl_clock *clock) {
    enum cm_wl_status status;
    uint8_t token;
    if(r->fault!=CM_WL_OK) return r->fault;
    if(r->in_callback!=0) return CM_WL_REENTER;
    token=lock(r);
    status=clock->overflow!=0 ? fault(r,CM_WL_SOURCE_OVERFLOW) : cm_wl_native_advance(r,clock->ticks);
    if(status==CM_WL_OK) clock->ticks=0;
    unlock(r,token); return status;
}
