/* SPDX-License-Identifier: MIT
 * Input fixtures exercise each WTLST5 CCS position with +0/-0/negative/
 * positive A, both end-marker and real displaced tasks, four task BBCONs. */
#include "waitlist_zero_cases.h"
void wl_zero_prepare(waitlist *w,uint16_t n,exec_address *task) {
 static const agc_word probe[]={0,AGC_NEG_ZERO,077776,1};
 uint8_t slot=(uint8_t)(n%8u),type=(uint8_t)((n/8u)%4u),i;
 w->time3=037754; w->ruptagn=AGC_NEG_ZERO; w->alarm=0;
 for(i=0;i<8;++i) w->interval[i]=i<slot ? 1 : i==slot ? probe[type] : (agc_word)(077700-i);
 for(i=0;i<9;++i) { w->task[i].location=i==8 && (n/32u)%2u==0 ? WL_END_LOCATION : (agc_word)(02200+i); w->task[i].bank=(agc_word)(012000+(i%8u)); }
 task->location=03000; task->bank=(agc_word)(022003+(n/64u)*020u);
}
void wl_zero_words(const waitlist *w,void (*emit)(void *,agc_word),void *u) {
 uint8_t i; emit(u,w->time3);
 for(i=0;i<8;++i) emit(u,w->interval[i]);
 for(i=0;i<9;++i) { emit(u,w->task[i].location); emit(u,w->task[i].bank); }
 emit(u,w->ruptagn); emit(u,w->alarm);
}
