/* SPDX-License-Identifier: MIT */
#ifndef INTERPRETER_NATIVE_CASES_H
#define INTERPRETER_NATIVE_CASES_H
#include "irq_delivery.h"
#define IN_CASE_COUNT 35u
#define IN_STEPS 16u
#define IN_LOW_BANK UINT16_C(06000)
#define IN_HIGH_BANK UINT16_C(040123)
#define IN_LOW_ENTRY UINT16_C(02200)
#define IN_LOW_A UINT16_C(02210)
#define IN_LOW_B UINT16_C(02212)
#define IN_LOW_SLEEP UINT16_C(02220)
#define IN_LOW_WAKE UINT16_C(02223)
#define IN_LOW_C UINT16_C(02230)
#define IN_LOW_FINISH UINT16_C(02232)
#define IN_HIGH_ENTRY UINT16_C(02300)
#define IN_HIGH_A UINT16_C(02310)
#define IN_HIGH_B UINT16_C(02312)
#define IN_HIGH_FINISH UINT16_C(02320)
#define IN_HIGH_BRIDGE UINT16_C(02322)
#define IN_HIGH_SLEEP UINT16_C(02324)
#define IN_HIGH_WAKE UINT16_C(02325)
#define IN_HIGH_PRIORITY UINT16_C(02326)
enum in_operation { IN_ADMIT,IN_START,IN_ENTER,IN_YIELD,IN_EXIT,IN_FINISH,IN_SLEEP,IN_WAKE,IN_PRIO };
typedef struct { enum in_operation operation; exec_address address;
                 agc_word priority; uint8_t touch; } in_event;
void in_prepare(executive *state,unsigned number);
unsigned in_steps(unsigned number);
void in_event_prepare(in_event *event,unsigned number,unsigned step);
void in_touch(executive *state,uint8_t touch);
void in_row(unsigned number,unsigned step,const executive *state,
            enum exec_status status,exec_address current,enum exec_context_kind kind,
            exec_interpreter control);
#ifndef IN_INPUT_ONLY
typedef struct {
    exec_native runtime;
    executive state;
    unsigned failures, callbacks, number;
    uint8_t frame_active, irq_enabled, expected_irq_mask;
    uint8_t (*interrupt_status)(void);
    exec_irq_queue queue;
    exec_exclusion guard;
} in_run;
void in_run_init(in_run *run,unsigned number);
enum exec_status in_run_step(in_run *run,unsigned step);
#endif
#endif
