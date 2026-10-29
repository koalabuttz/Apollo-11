/* SPDX-License-Identifier: MIT */
#ifndef CM_LIFECYCLE_CASES_H
#define CM_LIFECYCLE_CASES_H
#include "cm_native_dispatch.h"
#include "cm_dispatch_cases.h"
#define LIFECYCLE_CASE_COUNT 81
#define LIFECYCLE_CHECKPOINT_COUNT 115
#define LIFECYCLE_SEQUENCE_FIRST 76
enum cm_lifecycle_operation { LIFE_SLEEP, LIFE_WAKE, LIFE_PRIORITY, LIFE_ADMIT,
                           LIFE_START, LIFE_COMPLETE };
typedef struct {
    enum cm_lifecycle_operation operation;
    exec_address address;
    agc_word cadr,priority;
    uint8_t needs_vac,touch;
} cm_lifecycle_event;
void cm_lifecycle_prepare(cm_executive *s,uint8_t n);
uint8_t cm_lifecycle_steps(uint8_t n);
uint8_t cm_lifecycle_vector_index(uint8_t n,uint8_t step);
void cm_lifecycle_event_prepare(cm_lifecycle_event *e,uint8_t n,uint8_t step);
enum exec_status cm_lifecycle_apply(cm_executive *s,const cm_lifecycle_event *e,exec_address *next);
int cm_lifecycle_demo(cm_executive *s,uint8_t n,cm_dispatch_observer observer,void *user);
#endif
