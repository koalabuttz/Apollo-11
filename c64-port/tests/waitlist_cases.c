/* SPDX-License-Identifier: MIT */
#include "waitlist_cases.h"
static agc_word negative(unsigned magnitude) { return (agc_word)(magnitude ^ AGC_MASK); }
static const agc_word timer_cases[]={037777,030000,1,0,127,128};
static const agc_word interval_cases[]={1,0,AGC_NEG_ZERO,077776,WL_DEFAULT_INTERVAL,076777,040000,077600};
void wait_prepare(waitlist *s,uint16_t n) {
    uint8_t i;
    s->time3=037777; s->ruptagn=AGC_NEG_ZERO; s->alarm=0;
    for(i=0;i<8;++i) s->interval[i]=WL_DEFAULT_INTERVAL;
    for(i=0;i<9;++i) { s->task[i].location=WL_END_LOCATION; s->task[i].bank=WL_END_BANK; }
    if(n<90) {
        uint8_t variant=(uint8_t)(n%10);
        s->time3=(agc_word)(16384-20);
        for(i=0;i<8;++i) {
            s->task[i].location=(agc_word)(02200+i);
            s->task[i].bank=(agc_word)(06003+((i & 3u)<<4));
            s->interval[i]=negative(9);
        }
        if(variant==3 || variant==4) { s->task[8].location=02210; s->task[8].bank=040060; }
        if(variant==5) {
            for(i=0;i<8;++i) s->interval[i]=1; /* nine equal deadlines */
        }
        if(variant==6) {
            for(i=0;i<8;++i) s->interval[i]=AGC_NEG_ZERO; /* consecutive deadlines */
        }
        if(variant==7) { s->time3=0; s->interval[0]=1; }
        if(variant==8) { s->time3=1; s->interval[0]=0; }
        if(variant==9) { s->ruptagn=1; s->task[4]=s->task[3]; }
    } else if(n>=166) s->time3=timer_cases[(n-166)/5];
    else if(n>=156) s->time3=020000;
    else if(n<120) s->time3=timer_cases[(n-90)/5];
    else if(n<144) {
        s->time3=timer_cases[2+(n-120)/8];
        for(i=0;i<9;++i) { s->task[i].location=(agc_word)(02400+i); s->task[i].bank=(agc_word)(012000+(i & 7u)); }
        s->interval[0]=interval_cases[(n-120)%8];
        for(i=1;i<8;++i) s->interval[i]=negative((unsigned)i*37);
    } else if(n<152) {
        s->time3=timer_cases[(n-144)%4]; s->ruptagn=n<148 ? 1 : AGC_NEG_ZERO;
        s->task[0].location=02400; s->task[0].bank=046043; s->interval[0]=interval_cases[(n-144)%4];
    }
}
uint8_t wait_steps(uint16_t n) { return n>=156 && n<166 ? 12 : 1; }
void wait_event_prepare(wait_event *e,uint16_t n,uint8_t step) {
    e->operation=WAIT_INSERT; e->delay=1; e->address.location=03000; e->address.bank=022063; e->ticks=0;
    if(n>=166) {
        static const uint16_t ticks[]={0,1,127,16384,65535};
        e->operation=WAIT_TICK; e->ticks=ticks[(n-166)%5];
    } else if(n<90) {
        uint8_t slot=(uint8_t)(n/10),variant=(uint8_t)(n%10);
        e->delay=slot==0 ? 1 : (agc_word)(11+slot*10);
        if(variant==1) e->delay=(agc_word)(20+slot*10); /* equal deadlines */
        if(variant==2) e->delay=(agc_word)(19+slot*10); /* immediately before */
        if(variant==4) e->delay=150; /* full list: append fails before shifts */
        if(variant==5) e->delay=20;
        if(variant==6) e->delay=(agc_word)(20+slot);
        e->address.bank=(agc_word)(022000 | ((variant & 3u)<<4) | (slot & 7u));
    } else if(n<120) {
        static const agc_word delay[]={1,2,127,WL_MAX_DELAY,AGC_NEG_ZERO};
        e->delay=delay[(n-90)%5];
    } else if(n<144) e->operation=WAIT_BEGIN;
    else if(n<152) e->operation=WAIT_OVER;
    else if(n<156) {
        static const agc_word bad[]={0,AGC_NEG_ZERO,077776,040205};
        e->delay=bad[n-152];
    } else {
        uint16_t variant=(uint16_t)(n-156);
        uint16_t bias=(uint16_t)(variant*3u);
        if(step<3) {
            e->delay=(agc_word)(bias+(step<2 ? 3 : 5));
            e->address.location=(agc_word)(03000+step);
        } else if(step==3 || step==8) {
            e->operation=WAIT_TICK; e->ticks=step==3 ? (uint16_t)(bias+3) : (uint16_t)(variant%2 ? 3 : 2);
        } else if(step==4 || step==9) e->operation=WAIT_BEGIN;
        else if(step==6) { e->delay=1; e->address.location=03003; }
        else e->operation=WAIT_OVER;
    }
}
#ifndef WAIT_INPUT_ONLY
enum wl_status wait_apply(waitlist *s,const wait_event *e,exec_address *next) {
    switch(e->operation) {
    case WAIT_INSERT: return wl_schedule(s,e->delay,e->address);
    case WAIT_BEGIN: return wl_begin(s,next);
    case WAIT_OVER: return wl_taskover(s,next);
    default: return wl_advance(s,e->ticks)<0 ? WL_OUT_OF_SCOPE : WL_OK;
    }
}
static uint32_t digest_word(uint32_t h,agc_word w) {
    h=(h ^ (w & 255u))*UINT32_C(16777619);
    return (h ^ (w>>8))*UINT32_C(16777619);
}
uint32_t wait_digest(const waitlist *s,enum wl_status result,exec_address next) {
    uint8_t i;
    uint32_t h=UINT32_C(2166136261);
    h=digest_word(h,(agc_word)result); h=digest_word(h,s->time3);
    for(i=0;i<8;++i) h=digest_word(h,s->interval[i]);
    for(i=0;i<9;++i) { h=digest_word(h,s->task[i].location); h=digest_word(h,s->task[i].bank); }
    h=digest_word(h,s->ruptagn); h=digest_word(h,s->alarm);
    h=digest_word(h,next.location); return digest_word(h,next.bank);
}
#endif
#ifndef WAIT_INPUT_ONLY
typedef struct { uint16_t n,trace; wait_observe observe; void *user; } wait_demo;
static void observed(wl_native *r,wait_demo *d,uint8_t step,enum wl_status status,int transferred) {
    exec_address next={0,0};
    if(transferred && status==WL_OK) next=r->current;
    if(d->observe!=0) d->observe(d->user,d->n,step,r->state,status,next);
}
static enum wl_status task_a(wl_native *r,void *user) {
    wait_demo *d=user; (void)r;
    if(d->trace!=0) return WL_CALLBACK_FAULT;
    d->trace=1; return WL_OK;
}
static enum wl_status task_b(wl_native *r,void *user) {
    wait_demo *d=user; exec_address address={03003,022063}; enum wl_status status;
    if(d->trace!=1 || wl_native_step(r)!=WL_REENTER) return WL_CALLBACK_FAULT;
    d->trace=12; status=wl_native_schedule(r,1,address);
    observed(r,d,6,status,0); return status;
}
static enum wl_status task_c(wl_native *r,void *user) {
    wait_demo *d=user; (void)r;
    if(d->trace!=123) return WL_CALLBACK_FAULT;
    d->trace=1234; return WL_OK;
}
static enum wl_status task_d(wl_native *r,void *user) {
    wait_demo *d=user; (void)r;
    if(d->trace!=12) return WL_CALLBACK_FAULT;
    d->trace=123; return WL_OK;
}
static const wl_mapping task_mapping[]={
    {{03000,022063},task_a},{{03001,022063},task_b},{{03002,022063},task_c},{{03003,022063},task_d}
};
unsigned wait_native_run(waitlist *s,uint16_t n,const exec_exclusion *exclusion,wait_observe observe,void *user) {
    wl_native r;
    wait_demo d={n,0,observe,user};
    uint8_t step;
    unsigned bad=0;
    wait_prepare(s,n);
    if(wl_native_init(&r,s,task_mapping,4,&d,exclusion)!=WL_OK) return 1;
    for(step=0;step<12;++step) {
        wait_event e; enum wl_status status;
        if(step==6) continue; /* Observed inside the actual task-B callback. */
        wait_event_prepare(&e,n,step);
        if(e.operation==WAIT_INSERT) status=wl_native_schedule(&r,e.delay,e.address);
        else if(e.operation==WAIT_TICK) status=wl_native_advance(&r,e.ticks);
        else if(e.operation==WAIT_BEGIN) status=wl_native_start(&r);
        else status=wl_native_step(&r);
        if(status!=WL_OK && !(e.operation==WAIT_OVER && (step==7 || step==11) && status==WL_IDLE)) ++bad;
        observed(&r,&d,step,status,e.operation==WAIT_BEGIN || e.operation==WAIT_OVER);
    }
    if(d.trace!=1234 || r.running!=0 || r.pending!=0 || r.fault!=WL_OK) ++bad;
    return bad;
}
#endif
