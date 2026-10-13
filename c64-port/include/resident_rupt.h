/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_RUPT_H
#define RESIDENT_RUPT_H
#include "agc_transfer.h"
/* The flight resident's AGC timers and interrupt entry
 * (docs/RESIDENT-FLIGHT.md), as yaAGC's agc_engine.c models the hardware.
 *
 * rk_timer_tick is one 10 ms scaler period: TIME1 (025) PINC, its overflow
 * PINCs TIME2 (024); TIME3 (026), TIME4 (027) and TIME5 (030) PINC and an
 * overflow requests T3RUPT, T4RUPT or T5RUPT. While channel 13 bit 15 is set,
 * TIME6 (031) takes its sixteen 1/1600 s DINCs; a DINC of +0 or -0 (the
 * ZOUT) requests T6RUPT and clears channel 13 bit 15, which stops the clock.
 * TIME6 is exact in count but its interrupt lands at the end of the 10 ms
 * period, not at the 0.625 ms step.
 *
 * rk_rupt_take vectors the highest-priority pending request, as yaAGC
 * does before an instruction, when no interrupt is in progress, interrupts
 * are enabled (INHINT/RELINT, io's interrupt_enabled), A holds no overflow
 * and the word at Z is not RELINT, INHINT or EXTEND (3, 4, 6). Call it only
 * at a native boundary, where Z is the next instruction and no INDEX or
 * EXTEND is pending. It stores ZRUPT = Z + 1 and BRUPT = the word at Z, sets
 * Z to the vector (04000 + 4 * request) and program->hardware->in_interrupt,
 * clears that request and returns it; 0 when it takes none. The word at Z is
 * read from erasable (under EB from 01400) or, from 02000, through
 * program->fixed_data at agc_fixed_physical(Z, FB, superbank); a word it
 * cannot read takes nothing. */
enum {
    RK_RUPT_T6 = 1, RK_RUPT_T5, RK_RUPT_T3, RK_RUPT_T4, RK_RUPT_KEY1, RK_RUPT_KEY2,
    RK_RUPT_UP, RK_RUPT_DOWN, RK_RUPT_RADAR, RK_RUPT_HAND
};
#define RK_RUPT_BIT(request) ((uint16_t)(1u << (request)))
void rk_timer_tick(apollo_overlay_state *s, agc_hardware *hw, uint16_t *pending);
uint8_t rk_rupt_take(const agc_native_program *p, apollo_overlay_state *s, agc_word superbank,
                     agc_word interrupt_enabled, uint16_t *pending);
#endif
