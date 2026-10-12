/* SPDX-License-Identifier: MIT */
#ifndef LM_TIME_FREE_FALL_CASES_H
#define LM_TIME_FREE_FALL_CASES_H
#include "lm_time_free_fall.h"
/* Deterministic sampled original call chains of Luminary TIME_OF_FREE_FALL
 * as SR30.1 makes them. Case n is a chain of calls (TFFCONIC or TFFCONMU,
 * then TFFRP/RA, CALCTPER and CALCTFF in SR30.1's order); its state vector
 * is a sampled Earth or Moon orbit in integer arithmetic (circular speed
 * times a factor, a flight-path angle, a random plane) or, for one case in
 * eight, raw edge words, over poisoned ordinary erasable debris.
 * lm_time_free_fall_input builds the entry image of the first call;
 * lm_time_free_fall_call prepares call k (k > 0) on the image the previous
 * call returned, as SR30.1's CALL does (LOC, FBANK 27, QPRET, the loaded
 * MPAC). Both the oracle and the native producers use these. */
void lm_time_free_fall_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (erasable debris, A/L/Q, EB,
 * VAC, push-down pointer, OVFIND, IRQ/SUPER; tests/lm_time_free_fall_debris.c):
 * the base the frozen C64 phase steps are stored against. */
void lm_time_free_fall_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_time_free_fall_random(uint32_t *seed);
uint8_t lm_time_free_fall_calls(uint32_t n);
void lm_time_free_fall_call(apollo_overlay_state *,uint32_t n,uint8_t k);
#endif
