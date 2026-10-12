/* SPDX-License-Identifier: MIT */
#ifndef LM_MIDGIM_CASES_H
#define LM_MIDGIM_CASES_H
#include "lm_midgim.h"
/* Deterministic sampled original entries of the Luminary midcourse display
 * subroutines as their callers make them (case n by n%8: 0-2 GET+MGA, 3-5
 * GET.LVC, 6-7 SELECTMU), each at NEWOPS with QPRET one of the routine's
 * Luminary call returns (any caller's for SELECTMU). GET+MGA gets a pushed
 * velocity and a REFSMMAT rotation, GET.LVC a state vector and a delta V at
 * 0D; one case in eight has raw edge words, over poisoned ordinary
 * erasable debris (CMOONFLG and the other flags as the debris leaves them).
 * Both the oracle and the native producers use these. */
void lm_midgim_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* The orbit-free part of case n's entry image (tests/lm_midgim_debris.c):
 * the base the frozen C64 phase steps are stored against. */
void lm_midgim_debris(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_midgim_random(uint32_t *seed);
#endif
