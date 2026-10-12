/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_TIMETHET_CASES_H
#define LM_CONIC_TIMETHET_CASES_H
#include "lm_conic_timethet.h"
/* Deterministic sampled original entries of Luminary TIMETHET as its six
 * callers make them (CALL or STCALL TIMETHET with RVEC, VVEC, SNTH, CSTH
 * and RVSW set, X1 = -2 or -10). Case n is an Earth or Moon state vector
 * (circular speed times a factor, a flight-path angle that is sometimes
 * almost radial, a random plane) and a transfer angle theta (anywhere on
 * the circle, near 0, 180 or 360 degrees), or for one case in eight raw
 * edge words, over poisoned ordinary erasable debris. Both the oracle and
 * the native producers use these. */
void lm_conic_timethet_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (erasable debris, A/L/Q, EB,
 * VAC, OVFIND, IRQ/SUPER, X1, QPRET, RVSW; tests/lm_conic_timethet_debris.c):
 * the base the frozen C64 phase steps are stored against. */
void lm_conic_timethet_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_conic_timethet_random(uint32_t *seed);
#endif
