/* SPDX-License-Identifier: MIT */
#ifndef DSKY_KEY_IRQ_H
#define DSKY_KEY_IRQ_H
#include "dsky_keyrupt.h"
typedef struct {p66_bias_io control;uint16_t in_interrupt;} dsky_key_irq_io;
/* Complete original key interrupt vector4024, KEYRUPT1/KEYCOM, and source
 * RESUME through architecture return, before the interrupted source operation
 * executes again. Native caller resumes an explicit mapped C continuation;
 * BRUPT is preserved ordinary source data and is never decoded/executed here.
 * Canonical image includes interrupted register/bank state, ZRUPT/BRUPT and
 * real Executive words; caller excludes producers. All retained SUPER values
 * 0..0160 supported. Input in_interrupt=1; successful RESUME clears it.
 * Resource exhaustion stops actual BAILOUT with interrupt state retained. */
enum exec_status dsky_key_irq(apollo_overlay_state *,dsky_key_irq_io *,uint16_t channel15);
#endif
