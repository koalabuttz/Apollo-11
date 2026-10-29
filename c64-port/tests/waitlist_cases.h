/* SPDX-License-Identifier: MIT */
#ifndef WAITLIST_CASES_H
#define WAITLIST_CASES_H
#include "native_tasks.h"
#define WAIT_CASE_COUNT 196
#define WAIT_CHECKPOINT_COUNT 306
enum wait_operation { WAIT_INSERT, WAIT_TICK, WAIT_BEGIN, WAIT_OVER };
typedef struct { enum wait_operation operation; agc_word delay; exec_address address; uint16_t ticks; } wait_event;
void wait_prepare(waitlist *state,uint16_t n);
uint8_t wait_steps(uint16_t n);
void wait_event_prepare(wait_event *event,uint16_t n,uint8_t step);
typedef void (*wait_observe)(void *,uint16_t,uint8_t,const waitlist *,enum wl_status,exec_address);
#ifndef WAIT_INPUT_ONLY
unsigned wait_native_run(waitlist *state,uint16_t n,const exec_exclusion *exclusion,wait_observe observe,void *user);
enum wl_status wait_apply(waitlist *state,const wait_event *event,exec_address *next);
uint32_t wait_digest(const waitlist *state,enum wl_status result,exec_address next);
#endif
#endif
