/* SPDX-License-Identifier: MIT */
#ifndef DISPATCH_CASES_H
#define DISPATCH_CASES_H
#include "native_dispatch.h"
#define DISPATCH_CASE_COUNT 53
#define DISPATCH_SEQUENCE_FIRST 42
#define LOW_ENTRY 02010
#define LOW_RESUME 02011
#define LOW_FINISH 02012
#define HIGH_ENTRY 03010
#define HIGH_RESUME 03011
#define LOW_BANK 06022
#define HIGH_BANK 010104
enum dispatch_operation { DISPATCH_ADMIT, DISPATCH_START, DISPATCH_YIELD, DISPATCH_COMPLETE };
typedef struct {
    enum dispatch_operation operation;
    exec_address address;
    agc_word priority, superbank;
    uint8_t needs_vac, touch;
} dispatch_event;
uint8_t dispatch_vector_index(uint8_t n, uint8_t step);
void dispatch_prepare(executive *s, uint8_t n);
uint8_t dispatch_steps(uint8_t n);
void dispatch_event_prepare(dispatch_event *e, uint8_t n, uint8_t step);
void dispatch_touch(executive *s, uint8_t pattern);
enum exec_status dispatch_apply(executive *s, const dispatch_event *e, exec_address *next);
typedef void (*dispatch_observer)(void *, uint8_t, const executive *, enum exec_status, exec_address);
/* Actual native C callbacks, not a scripted call to dispatch_apply. */
int dispatch_demo(executive *s, uint8_t n, dispatch_observer observe, void *user);
#endif
