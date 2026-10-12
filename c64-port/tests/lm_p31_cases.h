/* SPDX-License-Identifier: MIT */
#ifndef LM_P31_CASES_H
#define LM_P31_CASES_H
#include "lm_p31_routines.h"
/* Deterministic sampled entries for the P31 transfer routines. Case n
 * enters entry n%17 over poisoned erasable debris: the eleven interpretive
 * entries at NEWOPS, then the six basic entries at Z (see
 * include/lm_p31_routines.h). Debris covers any vectors, times and flags,
 * every VAC, PUSHLOC, MODE, IRQ and allowed SUPERBNK, and EB (E7 three
 * times in four). Every case runs through the trampoline with this file's
 * routines to the first point outside them; the oracle stops at the same
 * point (lm_p31_stop). */
#define LM_P31_ROUTINES 17u
#define LM_P31_INTERPRETIVE 11u
#define LM_P31_PER_ROUTINE 512u
#define LM_P31_CASES (LM_P31_ROUTINES*LM_P31_PER_ROUTINE)
#define LM_P31_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_p31_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 35,2454-2544, P20FLGON, VNPOOH, INTPRET, or POODOO1
 * (alarm 1302). */
int lm_p31_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_P31_STOPPED 1u
#define LM_P31_REFUSED 2u
unsigned lm_p31_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_p31_digest(apollo_overlay_state *,uint32_t n);
#endif
