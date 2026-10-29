/* SPDX-License-Identifier: MIT */
#ifndef CM_WAITLIST_NATIVE_H
#define CM_WAITLIST_NATIVE_H
#include "cm_waitlist.h"
#include "irq_delivery.h"
struct cm_wl_native;
/* Runs with the bound producer excluded. Return CM_WL_OK to perform TASKOVER.
 * Persistent task data must outlive the call. No suspended task C stack. */
typedef enum cm_wl_status (*cm_wl_task_call)(struct cm_wl_native *,void *);
typedef struct { exec_address address; cm_wl_task_call call; } cm_wl_mapping;
typedef struct cm_wl_native {
    cm_waitlist *state;
    const cm_wl_mapping *mapping;
    const exec_exclusion *exclusion;
    void *user;
    exec_address current;
    uint8_t count,pending,running,in_callback;
    enum cm_wl_status fault;
} cm_wl_native;
/* Source stopped/excluded during init; storage/table/hooks must outlive runtime.
 * Dummy ENDTASK is a built-in no-op completion, excluding SVCT3's IMU body. */
enum cm_wl_status cm_wl_native_init(cm_wl_native *runtime,cm_waitlist *state,
    const cm_wl_mapping *mapping,uint8_t count,void *user,const exec_exclusion *exclusion);
enum cm_wl_status cm_wl_native_schedule(cm_wl_native *runtime,agc_word delay,exec_address task);
/* Platform feeds exact counted centiseconds at foreground safe points.
 * Counter publication/collection is platform-owned, never task dispatch in ISR. */
enum cm_wl_status cm_wl_native_advance(cm_wl_native *runtime,uint16_t ticks);
enum cm_wl_status cm_wl_native_start(cm_wl_native *runtime);
/* One selected task callback, then TASKOVER. Another due task can be selected
 * for the next step, with no Executive job exchange or live-frame preemption. */
enum cm_wl_status cm_wl_native_step(cm_wl_native *runtime);
/* One non-nested IRQ producer, foreground consumer under bound exclusion.
 * 16-bit publication is safe only with that mask/copy contract, not C threads. */
typedef struct { volatile uint16_t ticks; volatile uint8_t overflow; } cm_wl_clock;
void cm_wl_clock_init(cm_wl_clock *clock);
enum cm_wl_status cm_wl_clock_post(cm_wl_clock *clock);
/* Preserve pending ticks on failure; overflow fail-stops before TIME3 mutation.
 * Initialize/reset only with the source stopped. */
enum cm_wl_status cm_wl_native_collect(cm_wl_native *runtime,cm_wl_clock *clock);
#endif
