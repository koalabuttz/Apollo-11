/* SPDX-License-Identifier: MIT */
#ifndef C64_TIMER_PLATFORM_H
#define C64_TIMER_PLATFORM_H
#include "native_tasks.h"
extern const exec_exclusion c64_timer_exclusion;
uint8_t c64_timer_status(void);
/* Stock KERNAL vector/CIA1 Timer A owner; CIA1 B must be stopped at $ffff,
 * CRB=$08 and VIC raster IRQ disabled. Diagnostic PAL period is 19705 phi2 cycles (latch 19704).
 * Source publishes tick_step logical centiseconds per underflow, never tasks/jobs.
 * All IRQ-masked intervals plus handler delay must remain shorter than period:
 * CIA flags coalesce; missed underflows cannot be reconstructed by this adapter.
 * budget=0 is continuous; finite budget is a diagnostic, not a deadline proof. */
int c64_timer_install(wl_clock *clock,uint16_t period,uint8_t tick_step,uint8_t budget);
void c64_timer_remove(void);
extern volatile uint8_t c64_timer_count,c64_timer_done,c64_timer_fault;
extern volatile uint8_t c64_timer_poison,c64_timer_stack_fault;
uint8_t c64_timer_register_probe(void);
extern uint8_t c64_timer_stack[256];
#endif
