/* SPDX-License-Identifier: MIT */
#ifndef LM_CLOCK_H
#define LM_CLOCK_H
#include "overlay_runtime.h"
/* Hardware PINC, including the -0 -> +1 transition. Invalid raw words are
 * rejected before mutation. The return value is carry1, ordinary0, or -1. */
int lm_counter_pinc(uint16_t *word);
/* Explicit 10ms hardware pulses for TIME1/TIME2 and TIME3. The caller owns
 * producer exclusion and source IRQ admission. Pending is the real boolean
 * T3 request latch: overflows set it; this producer never clears it. No source
 * instructions, scheduling checks, or other scaler phases are performed. */
int lm_clock_advance(apollo_overlay_state *image,uint16_t centiseconds,
                     uint16_t *pending_t3);
#endif
