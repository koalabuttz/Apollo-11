/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_RUPT_H
#define RESIDENT_RUPT_H
#include "agc_transfer.h"
/* The flight resident's AGC timers and interrupt entry
 * (docs/RESIDENT-FLIGHT.md), as yaAGC's agc_engine.c models the hardware.
 *
 * rk_timer_tick is one 10 ms scaler period, as yaAGC steps it (SCALER1
 * 001-040) in three rk_timer_phase steps, each ending a part of the period:
 * - RK_PHASE_T4, 2.5 ms (SCALER1 010): TIME4 (027) PINC; its overflow
 *   requests T4RUPT.
 * - RK_PHASE_T3, at 5 ms (020): TIME1 (025) PINC, whose overflow PINCs
 *   TIME2 (024), then TIME3 (026); its overflow requests T3RUPT.
 * - RK_PHASE_T5, at 10 ms (040, the next period's 000): TIME5 (030); its
 *   overflow requests T5RUPT.
 * So when TIME4 and TIME3 overflow in one period, T4RUPT is requested
 * first, as on the AGC, and a clock that takes an interrupt between the
 * steps takes it first. SCALER1 (channel 4) counts 8, 8 and 16 and
 * SCALER2 (channel 3) its overflow. While channel 13 bit 15 is set, TIME6
 * (031) takes its 1/1600 s DINCs, 4, 4 and 8; a DINC of +0 or -0 (the ZOUT)
 * requests T6RUPT and clears channel 13 bit 15, which stops the clock.
 * TIME6 is exact in count but its interrupt lands at the end of the step,
 * not at the 0.625 ms DINC.
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
enum { RK_PHASE_T4, RK_PHASE_T3, RK_PHASE_T5 };
void rk_timer_phase(apollo_overlay_state *s, agc_hardware *hw, uint16_t *pending, uint8_t phase);
void rk_timer_tick(apollo_overlay_state *s, agc_hardware *hw, uint16_t *pending);
uint8_t rk_rupt_take(const agc_native_program *p, apollo_overlay_state *s, agc_word superbank,
                     agc_word interrupt_enabled, uint16_t *pending);
#endif
