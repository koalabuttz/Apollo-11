/* SPDX-License-Identifier: MIT */
#ifndef CM_IDLE_INTERRUPT_H
#define CM_IDLE_INTERRUPT_H
#include "cm_native_image.h"
/* Native control metadata equivalent to source InIsr/substitution. BRUPT is
 * retained in the canonical image but never decoded or executed natively. */
typedef struct {uint8_t in_isr,pending_idle;} cm_idle_interrupt_state;
enum cm_idle_interrupt_status {CM_IDLE_IRQ_ENTERED=0,CM_IDLE_IRQ_RESUMED=1,CM_IDLE_IRQ_DEFERRED=2,CM_IDLE_IRQ_INVALID=3};
/* Actual automatic T3 interrupt and original04014 lead-in through01,3423.
 * Certified source safepoint03231 is idle ADVAN's CCS NEWJOB. Interrupt delivery
 * defers unchanged for IRQ-disabled, ISR-active or overflowing A contexts. */
enum cm_idle_interrupt_status cm_idle_interrupt_enter(apollo_overlay_state *,cm_native_io *,cm_idle_interrupt_state *);
/* Actual hardware RESUME05341 to the compiled suspended idle ADVAN boundary.
 * The dispatcher must invoke its genuine compiled continuation next; no saved
 * instruction word is decoded and no artificial flight payload completes. */
enum cm_idle_interrupt_status cm_idle_interrupt_resume(apollo_overlay_state *,cm_native_io *,cm_idle_interrupt_state *);
#endif
