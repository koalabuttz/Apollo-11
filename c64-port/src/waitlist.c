/* SPDX-License-Identifier: MIT */
#include "waitlist.h"
static agc_register add(agc_register a,agc_word b) {
    return agc_add_register(a,agc_extend(b));
}
static agc_register complement(agc_register a) { return (agc_register)(a ^ UINT16_C(0177777)); }
static int reg_class(agc_register a) {
    int overflow=agc_overflow(a);
    if (overflow>0) return AGC_POSITIVE;
    if (overflow<0) return AGC_NEGATIVE;
    return agc_classify(agc_correct(a));
}
static agc_register reg_dabs(agc_register a) {
    agc_register magnitude=(a & UINT16_C(0100000))!=0 ? complement(a) : a;
    return magnitude==0 ? 0 : (agc_register)(magnitude-1u);
}
void wl_init(waitlist *s) {
    uint8_t i;
    s->time3=037777; s->ruptagn=AGC_NEG_ZERO; s->alarm=0;
    for(i=0;i<WL_INTERVAL_COUNT;++i) s->interval[i]=WL_DEFAULT_INTERVAL;
    for(i=0;i<WL_TASK_COUNT;++i) {
        s->task[i].location=WL_END_LOCATION; s->task[i].bank=WL_END_BANK;
    }
}
enum wl_status wl_schedule(waitlist *s,agc_word delay,exec_address task) {
    agc_register a;
    agc_word insert;
    uint8_t slot=0,i;
    int kind;
    if(s->alarm!=0) return WL_ALARM_PENDING;
    if(delay>AGC_MASK || s->time3>037777 || task.location==0 || task.location>07777 ||
       (task.bank & (agc_word)~UINT16_C(076167))!=0) return WL_OUT_OF_SCOPE;
    if(delay==0 || (delay & AGC_SIGN)!=0) { s->alarm=WL_BAD_DELAY; return WL_BAD_DELAY; }
    if(delay>WL_MAX_DELAY) return WL_OUT_OF_SCOPE;
    /* WAIT2 forms TD-T1+1, handling a pending TIME3 overflow (<0200).
     * Register arithmetic keeps the CCS zeros and uncorrected sign bits. */
    a=add(agc_extend(agc_negate(s->time3)),0200);
    kind=reg_class(a); a=reg_dabs(a);
    if(kind==AGC_POSITIVE) { a=add(a,040001); a=complement(a); }
    else if(kind==AGC_PLUS_ZERO) a=complement(a);
    if(kind!=AGC_MINUS_ZERO) a=add(a,040201);
    a=add(a,delay);
    kind=reg_class(a); a=reg_dabs(a);
    if(kind==AGC_POSITIVE) {
        a=add(a,s->interval[0]);
        /* WTLST5: equal deadlines insert after existing tasks. */
        for(slot=1;slot<WL_TASK_COUNT;++slot) {
            kind=reg_class(a); a=reg_dabs(a);
            if(kind==AGC_NEGATIVE || kind==AGC_MINUS_ZERO) break;
            if(slot==WL_INTERVAL_COUNT) break;
            a=add(a,s->interval[slot]);
        }
        if(slot==WL_TASK_COUNT || (slot==WL_INTERVAL_COUNT &&
           kind!=AGC_NEGATIVE && kind!=AGC_MINUS_ZERO)) {
            s->alarm=WL_FULL; return WL_FULL;
        }
        /* WTLST2: split the existing interval, preserving raw zero encoding. */
        if(kind==AGC_NEGATIVE) a=add(a,1);
        insert=agc_correct(complement(a));
        s->interval[slot-1]=agc_correct(add(add(a,1),s->interval[slot-1]));
    } else {
        /* Earlier than task 1: replace TIME3, insert old remaining difference. */
        a=add(add(agc_extend(agc_negate(delay)),020000),020000);
        insert=agc_correct(add(add(agc_extend(s->time3),040000),delay));
        s->time3=agc_correct(a); slot=0;
    }
    for(i=WL_INTERVAL_COUNT-1;i>slot;--i) s->interval[i]=s->interval[i-1];
    if(slot<WL_INTERVAL_COUNT) s->interval[slot]=insert;
    {
        exec_address discarded=s->task[WL_TASK_COUNT-1];
        for(i=WL_TASK_COUNT-1;i>slot;--i) s->task[i]=s->task[i-1];
        s->task[slot]=task;
        if(discarded.location!=WL_END_LOCATION) { s->alarm=WL_FULL; return WL_FULL; }
    }
    return WL_OK;
}
int wl_advance(waitlist *s,uint16_t ticks) {
    int due=0;
    if(s->alarm!=0 || s->time3>037777) return -1;
    while(ticks!=0) {
        if(s->time3==037777) { s->time3=0; due=1; }
        else ++s->time3;
        --ticks;
    }
    return due;
}
enum wl_status wl_begin(waitlist *s,exec_address *next) {
    uint8_t i;
    agc_register a;
    if(s->alarm!=0) return WL_ALARM_PENDING;
    if(next==0 || s->time3>037777 || s->task[0].location==0 || s->task[0].location>07777 ||
       (s->task[0].bank & (agc_word)~UINT16_C(076167))!=0) return WL_OUT_OF_SCOPE;
    a=add(add(agc_extend(s->interval[0]),037777),s->time3);
    s->time3=agc_correct(a);
    s->ruptagn=agc_overflow(a)>0 ? 1 : AGC_NEG_ZERO;
    for(i=0;i<WL_INTERVAL_COUNT-1;++i) s->interval[i]=s->interval[i+1];
    s->interval[WL_INTERVAL_COUNT-1]=WL_DEFAULT_INTERVAL;
    *next=s->task[0];
    for(i=0;i<WL_TASK_COUNT-1;++i) s->task[i]=s->task[i+1];
    s->task[WL_TASK_COUNT-1].location=WL_END_LOCATION;
    s->task[WL_TASK_COUNT-1].bank=WL_END_BANK;
    return WL_OK;
}
enum wl_status wl_taskover(waitlist *s,exec_address *next) {
    if(s->alarm!=0) return WL_ALARM_PENDING;
    if(next==0 || (s->ruptagn!=1 && s->ruptagn!=AGC_NEG_ZERO)) return WL_OUT_OF_SCOPE;
    if(s->ruptagn==1) return wl_begin(s,next);
    next->location=0; next->bank=0; return WL_IDLE;
}
