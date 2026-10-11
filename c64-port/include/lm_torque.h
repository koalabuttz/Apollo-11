/* SPDX-License-Identifier: MIT */
#ifndef LM_TORQUE_H
#define LM_TORQUE_H
#include "overlay_runtime.h"
typedef struct { uint16_t superbank, interrupt_enabled; } lm_torque_io;
#define LM_TORQUE_RATELOOP 03641u
#define LM_TORQUE_BACKP 02316u
#define LM_TORQUE_MINUS_100MST6 077537u
/* Complete original KALMAN_FILTER.agc RATELOOP through external BACKP entry.
 * One authoritative physical image; caller excludes producers for the entire
 * call. Source FB16, any internally consistent EB0..7 alias context, ZERO=0,
 * logical Z=RATELOOP, IRQ disabled; SUPER latch bits 020/040/100 may vary.
 * Ordinary image words are 15 bits; A/L/Q may contain raw 16-bit overflow.
 * No BACKP body, scheduler transition, hardware time or interrupt emulation.
 * Invalid inputs return zero without changing image or external latches. */
uint16_t lm_torque(apollo_overlay_state *, lm_torque_io *);
#endif
