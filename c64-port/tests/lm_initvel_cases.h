/* SPDX-License-Identifier: MIT */
#ifndef LM_INITVEL_CASES_H
#define LM_INITVEL_CASES_H
#include "lm_initvel.h"
/* Deterministic sampled original entries of Luminary INITVEL as its callers
 * make them (case n by n%8: 0-3 INITVEL, 4 HAVEGUES, 5 where INTSTALL
 * returns, 6-7 where INTEGRVS returns): RINIT and VINIT on an Earth or Moon
 * conic, RTARG where a nearby conic reaches after DELLT4, the iteration
 * count at 0D (usually zero, the conic case) and the cone angle at 2D, or
 * for one case in eight raw edge words, over poisoned ordinary erasable
 * debris. Both the oracle and the native producers use these. */
void lm_initvel_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (erasable debris, A/L/Q, EB,
 * VAC, PUSHLOC, OVFIND, IRQ/SUPER, RTX1/RTX2, QPRET, NORMEX, FBANK and LOC;
 * tests/lm_initvel_debris.c): the base the frozen C64 phase steps are
 * stored against. */
void lm_initvel_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_initvel_random(uint32_t *seed);
#endif
