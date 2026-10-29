/* SPDX-License-Identifier: MIT */
#ifndef IRQ_DELIVERY_H
#define IRQ_DELIVERY_H
#include "native_dispatch.h"
#define EXEC_IRQ_CAPACITY 4
/* Platform restores the caller's previous IRQ-enable state; guards may nest.
 * Hooks must exclude the single IRQ producer, not merely be compiler barriers. */
typedef struct exec_exclusion {
    uint8_t (*enter)(void *);
    void (*leave)(void *, uint8_t);
    void *user;
} exec_exclusion;
typedef struct {
    agc_word priority;
    exec_address address;
    uint8_t needs_vac;
} exec_request;
/* Single non-nested IRQ producer, foreground consumer. All publications are
 * volatile, byte-sized tickets; target mask/copy guards supply the synchronization.
 * Not a C11-thread/lock-free queue. It buffers admission requests, not ready jobs. */
typedef struct exec_irq_queue {
    volatile exec_request item[EXEC_IRQ_CAPACITY];
    volatile uint8_t write, read, overflow, rejected;
} exec_irq_queue;
/* Initialize/reset only while the source is stopped/excluded. */
void exec_irq_init(exec_irq_queue *queue);
/* IRQ-only producer: never calls Executive or changes running job state.
 * Full queue latches overflow, preserving accepted entries; further posts fail.
 * Invalid inputs are rejected without publishing. Source must report errors. */
enum exec_status exec_irq_post(exec_irq_queue *queue, const exec_request *request);
/* Bind before running, with source stopped; queue/hooks/storage must outlive r. */
enum exec_status exec_native_bind_irq(exec_native *runtime, exec_irq_queue *queue,
                                     const exec_exclusion *exclusion);
/* Internal guarded operations shared with the native trampoline. */
uint8_t exec_native_lock(exec_native *runtime);
void exec_native_unlock(exec_native *runtime, uint8_t token);
enum exec_status exec_irq_service_locked(exec_native *runtime);
#endif
