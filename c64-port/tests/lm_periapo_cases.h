/* SPDX-License-Identifier: MIT */
#ifndef LM_PERIAPO_CASES_H
#define LM_PERIAPO_CASES_H
#include "lm_periapo.h"
/* Deterministic sampled original entries of the Luminary apsides display
 * subroutines as their callers make them (case n by n%8: 0-2 PERIAPO1, 3
 * PERIAPO, 4 VECSHIFT, 5 SHIFTR1, 6 MAXCHK, 7 LEMSTORE or CSMSTORE), each
 * at NEWOPS with QPRET one of that routine's Luminary call returns. The
 * state vectors are Earth or Moon conics (circular speed times a factor,
 * sometimes exactly circular or hyperbolic, a flight-path angle, a random
 * plane) or, for one case in eight, raw edge words, over poisoned ordinary
 * erasable debris. Both the oracle and the native producers use these. */
void lm_periapo_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (erasable debris, A/L/Q, EB,
 * VAC, PUSHLOC, MODE, OVFIND, IRQ/SUPER, X1, RTX1/RTX2, QPRET, FBANK and
 * LOC; tests/lm_periapo_debris.c): the base the frozen C64 phase steps are
 * stored against. */
void lm_periapo_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_periapo_random(uint32_t *seed);
#endif
