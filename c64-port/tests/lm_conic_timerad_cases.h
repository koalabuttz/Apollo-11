/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_TIMERAD_CASES_H
#define LM_CONIC_TIMERAD_CASES_H
#include "lm_conic_timerad.h"
/* Deterministic sampled original entries of Luminary TIMERAD (even n) and
 * APSIDES (odd n) as their callers make them: P32/P72 CSI's STCALL
 * RDESIRED TIMERAD (RVSW set, RDESIRED = DPPOSMAX there; sampled also at
 * radii inside, near and outside the orbit's range, with either SGNRDOT)
 * and PERIAPO's STCALL XXXALT APSIDES, X1 = -2 or -10. Case n is an Earth
 * or Moon state vector (circular speed times a factor, sometimes exactly
 * circular or hyperbolic, a flight-path angle, a random plane), or for one
 * case in eight raw edge words, over poisoned ordinary erasable debris.
 * Both the oracle and the native producers use these. */
void lm_conic_timerad_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (erasable debris, A/L/Q, EB,
 * VAC, OVFIND, IRQ/SUPER, X1, QPRET, LOC, RVSW;
 * tests/lm_conic_timerad_debris.c): the base the frozen C64 phase steps are
 * stored against. */
void lm_conic_timerad_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_conic_timerad_random(uint32_t *seed);
#endif
