/* SPDX-License-Identifier: MIT */
#ifndef C64_IRQ_PLATFORM_H
#define C64_IRQ_PLATFORM_H
#include "irq_delivery.h"
extern const exec_exclusion c64_exclusion;
uint8_t c64_irq_status(void);
/* Bounded raster source; requires stock KERNAL IRQ vector and no raster owner.
 * Caller must keep queue/request lifetime valid until removal. No NMI support. */
int c64_irq_install(exec_irq_queue *queue, const exec_request *request, uint8_t budget);
void c64_irq_remove(void);
/* Arm while raster source is quiescent; at most budget<=5 IRQs are produced.
 * 5 permits the intentional four-entry overflow test. */
void c64_irq_arm(uint8_t budget);
extern volatile uint8_t c64_irq_count, c64_irq_done, c64_irq_last_status;
extern volatile uint8_t c64_irq_poison;
extern volatile uint16_t c64_irq_arithmetic;
extern volatile uint8_t c64_irq_stack_fault;
extern uint8_t c64_irq_stack[256];
uint8_t c64_register_probe(void);
#endif
