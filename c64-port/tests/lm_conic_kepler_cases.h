/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_KEPLER_CASES_H
#define LM_CONIC_KEPLER_CASES_H
#include "lm_conic_kepler.h"
/* Deterministic sampled original entries of Luminary KEPLERN as KEPPREP
 * makes them (GOTO KEPLERN with X1 = -2 or -10 and KEPRTN set). Case n is
 * an Earth or Moon state vector (circular speed times a factor, a flight-
 * path angle, a random plane), a transfer time TAU. (within a fraction of
 * a period, several periods, or hyperbolic hours) and an initial guess
 * XKEPNEW (near the solution, poor, zero, of the wrong sign or beyond the
 * bound), with XPREV/TC, or for one case in eight raw edge words, over
 * poisoned ordinary erasable debris. Both the oracle and the native
 * producers use these. */
void lm_conic_kepler_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (erasable debris, A/L/Q, EB,
 * VAC, OVFIND, IRQ/SUPER, X1, KEPRTN; tests/lm_conic_kepler_debris.c): the
 * base the frozen C64 phase steps are stored against. */
void lm_conic_kepler_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_conic_kepler_random(uint32_t *seed);
#endif
