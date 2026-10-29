/* SPDX-License-Identifier: MIT */
#ifndef CM_IRQ_H
#define CM_IRQ_H
#include "cm_native_dispatch.h"
#include "irq_delivery.h" /* Shared platform exclusion contract only. */
#define CM_IRQ_CAPACITY 4
#define CM_IRQ_MAP_CAPACITY 16
typedef struct { agc_word priority; exec_address address; uint8_t needs_vac; } cm_irq_request;
typedef struct {
 volatile cm_irq_request item[CM_IRQ_CAPACITY];
 volatile uint8_t write,read,overflow,rejected;
} cm_irq_queue;
/* The adapter owns tickets/native metadata, never a second Executive word image.
 * Immutable original tables and caller's runtime/state must outlive it. Bind with source stopped.
 * One non-nested ISR publishes payload before its byte-sized ticket; foreground
 * copies/admission/exchanges occur under restoring exclusion, callbacks outside.
 * Not a C11 thread queue. Call the adapter API after binding. */
typedef struct cm_irq_runtime {
 cm_exec_native *native; cm_irq_queue *queue; const exec_exclusion *exclusion;
 const cm_exec_mapping *original_basic;
 const cm_exec_context_mapping *original_context;
 void *original_user;
 union { cm_exec_mapping basic[CM_IRQ_MAP_CAPACITY];
         cm_exec_context_mapping context[CM_IRQ_MAP_CAPACITY]; } proxy;
 uint8_t token,held;
} cm_irq_runtime;
void cm_irq_init(cm_irq_queue *);
enum exec_status cm_irq_post(cm_irq_queue *,const cm_irq_request *);
enum exec_status cm_irq_bind(cm_irq_runtime *,cm_exec_native *,cm_irq_queue *,const exec_exclusion *);
enum exec_status cm_irq_poll(cm_irq_runtime *);
enum exec_status cm_irq_start(cm_irq_runtime *);
enum exec_status cm_irq_step(cm_irq_runtime *);
enum exec_status cm_irq_admit(cm_irq_runtime *,agc_word,exec_address,int);
enum exec_status cm_irq_wake(cm_irq_runtime *,agc_word);
#endif
