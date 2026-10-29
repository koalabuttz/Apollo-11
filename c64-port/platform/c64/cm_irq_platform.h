/* SPDX-License-Identifier: MIT */
#ifndef CM_C64_IRQ_PLATFORM_H
#define CM_C64_IRQ_PLATFORM_H
#include "cm_irq.h"
extern const exec_exclusion cm_c64_exclusion;
uint8_t cm_c64_irq_status(void);
/* Bounded raster source; requires stock KERNAL IRQ vector and no raster owner.
 * Caller must keep queue/request lifetime valid until removal. No NMI support. */
int cm_c64_irq_install(cm_irq_queue *queue, const cm_irq_request *request, uint8_t budget);
void cm_c64_irq_remove(void);
/* Arm while raster source is quiescent; at most budget<=5 IRQs are produced.
 * 5 permits the intentional four-entry overflow test. */
void cm_c64_irq_arm(uint8_t budget);
extern volatile uint8_t cm_c64_irq_count, cm_c64_irq_done, cm_c64_irq_last_status;
extern volatile uint8_t cm_c64_irq_poison;
extern volatile uint16_t cm_c64_irq_arithmetic;
extern volatile uint8_t cm_c64_irq_stack_fault;
extern uint8_t cm_c64_irq_stack[256];
uint8_t cm_c64_register_probe(void);
#endif
