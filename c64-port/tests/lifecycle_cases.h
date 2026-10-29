/* SPDX-License-Identifier: MIT */
#ifndef LIFECYCLE_CASES_H
#define LIFECYCLE_CASES_H
#include "irq_delivery.h"
#include "dispatch_cases.h"
#define LIFECYCLE_CASE_COUNT 81
#define LIFECYCLE_CHECKPOINT_COUNT 115
#define LIFECYCLE_SEQUENCE_FIRST 76
enum lifecycle_operation { LIFE_SLEEP, LIFE_WAKE, LIFE_PRIORITY, LIFE_ADMIT,
                           LIFE_START, LIFE_COMPLETE };
typedef struct {
    enum lifecycle_operation operation;
    exec_address address;
    agc_word cadr,priority;
    uint8_t needs_vac,touch;
} lifecycle_event;
void lifecycle_prepare(executive *s,uint8_t n);
uint8_t lifecycle_steps(uint8_t n);
uint8_t lifecycle_vector_index(uint8_t n,uint8_t step);
void lifecycle_event_prepare(lifecycle_event *e,uint8_t n,uint8_t step);
enum exec_status lifecycle_apply(executive *s,const lifecycle_event *e,exec_address *next);
int lifecycle_demo(executive *s,uint8_t n,dispatch_observer observer,void *user,
                   const exec_exclusion *exclusion);
#endif
