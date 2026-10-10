/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "waitlist_check.h"
static unsigned errors;
#define CHECK(c) do { if(!(c)) { fprintf(stderr,"waitlist check failed line %d: %s\n",__LINE__,#c); ++errors; } } while(0)
static uint8_t masked;
static unsigned enters,leaves;
static uint8_t enter(void *user) { uint8_t old=masked; (void)user; masked=1; ++enters; return old; }
static void leave(void *user,uint8_t old) { (void)user; CHECK(masked==1); masked=old; ++leaves; }
static const exec_exclusion guard={enter,leave,0};
static enum wl_status callback(wl_native *r,void *user) {
    unsigned *calls=user;
    CHECK(masked==1); ++*calls;
    CHECK(wl_native_step(r)==WL_REENTER && wl_native_start(r)==WL_REENTER);
    CHECK(wl_native_advance(r,1)==WL_REENTER);
    return WL_OK;
}
static enum wl_status faulty(wl_native *r,void *user) {
    exec_address a={03000,022063}; (void)user;
    CHECK(wl_native_schedule(r,0,a)==WL_BAD_DELAY);
    return WL_OK; /* Helper fault must block TASKOVER even if callback returns OK. */
}
int main(void) {
    waitlist s,before;
    wl_native r;
    wl_clock clock;
    unsigned calls=0;
    exec_address a={03000,022063},next={01234,06003};
    wl_mapping mapping[]={{ {03000,022063},callback },{ {03001,022063},faulty }};
    uint16_t i;
    errors+=waitlist_check(&s,0);
    masked=0; errors+=waitlist_check(&s,&guard); CHECK(masked==0 && enters==leaves);
    masked=1; errors+=waitlist_check(&s,&guard); CHECK(masked==1 && enters==leaves); masked=0;
    wl_init(&s); before=s;
    CHECK(wl_schedule(&s,16251,a)==WL_OUT_OF_SCOPE && memcmp(&s,&before,sizeof s)==0);
    CHECK(wl_schedule(&s,1,(exec_address){0,0})==WL_OUT_OF_SCOPE && memcmp(&s,&before,sizeof s)==0);
    CHECK(wl_schedule(&s,1,(exec_address){03000,010})==WL_OUT_OF_SCOPE);
    CHECK(wl_begin(&s,0)==WL_OUT_OF_SCOPE);
    s.task[0].location=040000; before=s;
    CHECK(wl_begin(&s,&next)==WL_OUT_OF_SCOPE && memcmp(&s,&before,sizeof s)==0 && next.location==01234);
    s.ruptagn=0; CHECK(wl_taskover(&s,&next)==WL_OUT_OF_SCOPE);
    wl_init(&s); s.time3=040000; before=s;
    CHECK(wl_advance(&s,1)==-1 && memcmp(&s,&before,sizeof s)==0);
    /* All positive timer encodings cross the actual PINC overflow boundary. */
    for(i=0;i<16384;++i) {
        wl_init(&s); s.time3=i;
        CHECK(wl_advance(&s,(uint16_t)(16384-i))==1 && s.time3==0);
    }
    wl_init(&s); s.time3=020000;
    CHECK(wl_native_init(&r,&s,mapping,2,&calls,&guard)==WL_OK);
    CHECK(wl_native_start(&r)==WL_IDLE && masked==0);
    CHECK(wl_native_schedule(&r,1,a)==WL_OK && masked==0);
    CHECK(wl_native_advance(&r,1)==WL_OK && r.pending==1);
    CHECK(wl_native_start(&r)==WL_OK);
    CHECK(wl_native_step(&r)==WL_IDLE && calls==1 && masked==0);
    /* Hardware-style pending overflow cannot be ignored beyond 127cs. */
    wl_init(&s); CHECK(wl_native_init(&r,&s,mapping,2,&calls,&guard)==WL_OK);
    before=s; CHECK(wl_native_advance(&r,129)==WL_SOURCE_OVERFLOW);
    CHECK(memcmp(&s,&before,sizeof s)==0 && wl_native_start(&r)==WL_SOURCE_OVERFLOW && masked==0);
    wl_init(&s); CHECK(wl_native_init(&r,&s,mapping,2,&calls,&guard)==WL_OK);
    CHECK(wl_native_advance(&r,128)==WL_OK && s.time3==127);
    before=s; CHECK(wl_native_advance(&r,1)==WL_SOURCE_OVERFLOW && memcmp(&s,&before,sizeof s)==0);
    wl_init(&s); s.time3=020000;
    CHECK(wl_native_init(&r,&s,mapping,2,&calls,&guard)==WL_OK);
    CHECK(wl_native_schedule(&r,1,mapping[1].address)==WL_OK);
    CHECK(wl_native_advance(&r,1)==WL_OK && wl_native_start(&r)==WL_OK);
    before=s; CHECK(wl_native_step(&r)==WL_BAD_DELAY && s.alarm==WL_BAD_DELAY);
    CHECK(s.time3==before.time3 && memcmp(s.task,before.task,sizeof s.task)==0 && masked==0);
    wl_init(&s); CHECK(wl_native_init(&r,&s,mapping,2,&calls,&guard)==WL_OK);
    before=s; CHECK(wl_native_schedule(&r,1,(exec_address){03000,022060})==WL_UNMAPPED);
    CHECK(memcmp(&s,&before,sizeof s)==0 && wl_native_step(&r)==WL_UNMAPPED);
    mapping[1]=mapping[0]; CHECK(wl_native_init(&r,&s,mapping,2,&calls,&guard)==WL_OUT_OF_SCOPE);
    CHECK(wl_native_init(0,&s,mapping,2,&calls,0)==WL_OUT_OF_SCOPE);
    CHECK(wl_native_init(&r,&s,mapping,1,&calls,&(exec_exclusion){0,leave,0})==WL_OUT_OF_SCOPE);
    wl_init(&s); s.time3=020000;
    CHECK(wl_native_init(&r,&s,mapping,1,&calls,&guard)==WL_OK);
    for(i=0;i<9;++i) CHECK(wl_native_schedule(&r,1,a)==WL_OK);
    CHECK(wl_native_schedule(&r,1,a)==WL_FULL && s.alarm==WL_FULL);
    CHECK(wl_native_step(&r)==WL_FULL && masked==0);
    wl_init(&s); s.time3=020000;
    CHECK(wl_native_init(&r,&s,mapping,1,&calls,&guard)==WL_OK);
    wl_clock_init(&clock);
    for(i=0;i<5;++i) CHECK(wl_clock_post(&clock)==WL_OK);
    CHECK(wl_native_collect(&r,&clock)==WL_OK && clock.ticks==0 && s.time3==020005 && masked==0);
    wl_clock_init(&clock); clock.ticks=UINT16_MAX;
    CHECK(wl_clock_post(&clock)==WL_SOURCE_OVERFLOW && clock.ticks==UINT16_MAX && clock.overflow==1);
    before=s; CHECK(wl_native_collect(&r,&clock)==WL_SOURCE_OVERFLOW && memcmp(&s,&before,sizeof s)==0);
    CHECK(clock.ticks==UINT16_MAX && masked==0);
    CHECK(enters==leaves);
    if(errors!=0) return 1;
    puts("PASS: 196 WAITLIST cases / 306 original checkpoints, ten native task sequences, clock wrap, restoring guards and fail-stop tests");
    return 0;
}
