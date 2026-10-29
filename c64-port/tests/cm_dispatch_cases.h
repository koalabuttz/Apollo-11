/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPATCH_CASES_H
#define CM_DISPATCH_CASES_H
#include "cm_native_dispatch.h"
#define DISPATCH_CASE_COUNT 53
#define DISPATCH_SEQUENCE_FIRST 42
#define LOW_ENTRY 02010
#define LOW_RESUME 02011
#define LOW_FINISH 02012
#define HIGH_ENTRY 03010
#define HIGH_RESUME 03011
#define LOW_BANK 06022
#define HIGH_BANK 010104
enum cm_dispatch_operation { DISPATCH_ADMIT, DISPATCH_START, DISPATCH_YIELD, DISPATCH_COMPLETE };
typedef struct {
    enum cm_dispatch_operation operation;
    exec_address address;
    agc_word priority, superbank;
    uint8_t needs_vac, touch;
} cm_dispatch_event;
uint8_t cm_dispatch_vector_index(uint8_t n, uint8_t step);
void cm_dispatch_prepare(cm_executive *s, uint8_t n);
uint8_t cm_dispatch_steps(uint8_t n);
void cm_dispatch_event_prepare(cm_dispatch_event *e, uint8_t n, uint8_t step);
void cm_dispatch_touch(cm_executive *s, uint8_t pattern);
enum exec_status cm_dispatch_apply(cm_executive *s, const cm_dispatch_event *e, exec_address *next);
typedef void (*cm_dispatch_observer)(void *, uint8_t, const cm_executive *, enum exec_status, exec_address);
/* Actual native C callbacks, not a scripted call to cm_dispatch_apply. */
int cm_dispatch_demo(cm_executive *s, uint8_t n, cm_dispatch_observer observe, void *user);
#endif
