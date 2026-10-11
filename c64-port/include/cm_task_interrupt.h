/* SPDX-License-Identifier: MIT */
#ifndef CM_TASK_INTERRUPT_H
#define CM_TASK_INTERRUPT_H
#include "cm_native_image.h"
/* Native control metadata equivalent to source InIsr/substitution. BRUPT is
 * retained in the canonical image but never decoded or executed natively. */
typedef struct {uint8_t in_isr,pending_danzig;} cm_task_interrupt_state;
enum cm_task_interrupt_status {CM_TASK_IRQ_ENTERED=0,CM_TASK_IRQ_RESUMED=1,CM_TASK_IRQ_DEFERRED=2,CM_TASK_IRQ_INVALID=3};
/* Actual automatic T3 interrupt and original04014 lead-in through01,3423.
 * Certified source safepoint06030 is DANZIG's CA BANKSET. Interrupt delivery
 * defers unchanged for IRQ-disabled, ISR-active or overflowing A contexts. */
enum cm_task_interrupt_status cm_task_interrupt_enter(apollo_overlay_state *,cm_native_io *,cm_task_interrupt_state *);
/* Actual hardware RESUME05341 to the compiled suspended DANZIG boundary.
 * The dispatcher must invoke its genuine compiled continuation next; no saved
 * instruction word is decoded and no artificial flight payload completes. */
enum cm_task_interrupt_status cm_task_interrupt_resume(apollo_overlay_state *,cm_native_io *,cm_task_interrupt_state *);
#endif
