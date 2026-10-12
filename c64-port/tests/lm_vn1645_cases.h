/* SPDX-License-Identifier: MIT */
#ifndef LM_VN1645_CASES_H
#define LM_VN1645_CASES_H
#include "lm_vn1645.h"
/* Deterministic sampled original entries of Luminary VN1645 as its
 * callers' GOTO makes them (case n by n%8: 0 FINALFLG clear, 1 REFSMFLG
 * clear, 2 a P7X program, 3-7 a P3X program, 6 any MODREG), at NEWOPS with
 * DELVSIN and a REFSMMAT rotation; one case in eight has raw edge words,
 * over poisoned ordinary erasable debris. Both the oracle and the native
 * producers use these. */
void lm_vn1645_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (tests/lm_vn1645_debris.c):
 * the base the frozen C64 phase steps are stored against. */
void lm_vn1645_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_vn1645_random(uint32_t *seed);
#endif
