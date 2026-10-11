/* SPDX-License-Identifier: MIT */
#ifndef CM_WAITLIST_CASES_H
#define CM_WAITLIST_CASES_H
#include "cm_waitlist_native.h"
#define CM_WAIT_CASE_COUNT 196
#define CM_WAIT_CHECKPOINT_COUNT 306
enum cm_wait_operation { CM_WAIT_INSERT, CM_WAIT_TICK, CM_WAIT_BEGIN, CM_WAIT_OVER };
typedef struct { enum cm_wait_operation operation; agc_word delay; exec_address address; uint16_t ticks; } cm_wait_event;
void cm_wait_prepare(cm_waitlist *state,uint16_t n);
uint8_t cm_wait_steps(uint16_t n);
void cm_wait_event_prepare(cm_wait_event *event,uint16_t n,uint8_t step);
typedef void (*cm_wait_observe)(void *,uint16_t,uint8_t,const cm_waitlist *,enum cm_wl_status,exec_address);
#ifndef CM_WAIT_INPUT_ONLY
unsigned cm_wait_native_run(cm_waitlist *state,uint16_t n,const exec_exclusion *exclusion,cm_wait_observe observe,void *user);
enum cm_wl_status cm_wait_apply(cm_waitlist *state,const cm_wait_event *event,exec_address *next);
uint32_t cm_wait_digest(const cm_waitlist *state,enum cm_wl_status result,exec_address next);
#endif
#endif
