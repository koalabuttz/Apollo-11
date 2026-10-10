/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_TASKS_H
#define NATIVE_TASKS_H
#include "waitlist.h"
#include "irq_delivery.h"
struct wl_native;
/* Runs with the bound producer excluded. Return WL_OK to perform TASKOVER.
 * Persistent task data must outlive the call. No suspended task C stack. */
typedef enum wl_status (*wl_task_call)(struct wl_native *,void *);
typedef struct { exec_address address; wl_task_call call; } wl_mapping;
typedef struct wl_native {
    waitlist *state;
    const wl_mapping *mapping;
    const exec_exclusion *exclusion;
    void *user;
    exec_address current;
    uint8_t count,pending,running,in_callback;
    enum wl_status fault;
} wl_native;
/* Source stopped/excluded during init; storage/table/hooks must outlive runtime.
 * Dummy ENDTASK is a built-in no-op completion, excluding SVCT3's IMU body. */
enum wl_status wl_native_init(wl_native *runtime,waitlist *state,
    const wl_mapping *mapping,uint8_t count,void *user,const exec_exclusion *exclusion);
enum wl_status wl_native_schedule(wl_native *runtime,agc_word delay,exec_address task);
/* Platform feeds exact counted centiseconds at foreground safe points.
 * Counter publication/collection is platform-owned, never task dispatch in ISR. */
enum wl_status wl_native_advance(wl_native *runtime,uint16_t ticks);
enum wl_status wl_native_start(wl_native *runtime);
/* One selected task callback, then TASKOVER. Another due task can be selected
 * for the next step, with no Executive job exchange or live-frame preemption. */
enum wl_status wl_native_step(wl_native *runtime);
/* One non-nested IRQ producer, foreground consumer under bound exclusion.
 * 16-bit publication is safe only with that mask/copy contract, not C threads. */
typedef struct { volatile uint16_t ticks; volatile uint8_t overflow; } wl_clock;
void wl_clock_init(wl_clock *clock);
enum wl_status wl_clock_post(wl_clock *clock);
/* Preserve pending ticks on failure; overflow fail-stops before TIME3 mutation.
 * Initialize/reset only with the source stopped. */
enum wl_status wl_native_collect(wl_native *runtime,wl_clock *clock);
#endif
