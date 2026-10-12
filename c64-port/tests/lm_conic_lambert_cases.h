/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_LAMBERT_CASES_H
#define LM_CONIC_LAMBERT_CASES_H
#include "lm_conic_lambert.h"
/* Deterministic sampled original entries of Luminary LAMBERT as INITVEL
 * makes them: CALL SETITCTR (ITERCTR 20, or 5 with AVEGFLAG set) and on to
 * LAMBERT with R1VEC, R2VEC, TDESIRED, GEOMSGN, VTARGTAG, GUESSW (with a
 * COGA guess when clear) and NORMSW (with UN when set), X1 = -2 or -10.
 * Case n is a pair of positions on an Earth or Moon conic and the time
 * between them (sometimes perturbed, sometimes unrelated or the same
 * direction), or for one case in eight raw edge words, over poisoned
 * ordinary erasable debris. Both the oracle and the native producers use
 * these. */
void lm_conic_lambert_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (erasable debris, A/L/Q, EB,
 * VAC, OVFIND, IRQ/SUPER, X1, QPRET, ITERCTR, LOC, GUESSW, NORMSW;
 * tests/lm_conic_lambert_debris.c): the base the frozen C64 phase steps are
 * stored against. */
void lm_conic_lambert_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_conic_lambert_random(uint32_t *seed);
#endif
