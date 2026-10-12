/* SPDX-License-Identifier: MIT */
#ifndef LM_P33_CASES_H
#define LM_P33_CASES_H
#include "lm_p33_routines.h"
/* Deterministic sampled entries for the P33/P73, AVFLAGA, AVFLAGP,
 * P20FLGON and DISDVLVC transfer routines. Case n enters entry n%43 over
 * poisoned erasable debris: the twenty interpretive entries at NEWOPS, then
 * the twenty-three basic entries at Z (see include/lm_p33_routines.h).
 * Debris covers any times, vectors and flags (FINALFLG), MPAC zero in one
 * S33/34.1 return in two (both BZE outcomes), every VAC, PUSHLOC, MODE, IRQ
 * and allowed SUPERBNK, EB (E7 three times in four), and a CADR in banks
 * 2-27 in NORMEX. Every case runs through the trampoline with this file's
 * routines to the first point outside them; the oracle stops at the same
 * point (lm_p33_stop). */
#define LM_P33_ROUTINES 43u
#define LM_P33_INTERPRETIVE 20u
#define LM_P33_PER_ROUTINE 512u
#define LM_P33_CASES (LM_P33_ROUTINES*LM_P33_PER_ROUTINE)
#define LM_P33_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_p33_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 35,2225-2265, 2277-2346, 2370-2402 and 2406-2407, UPFLAG,
 * DOWNFLAG, VNPOOH, ALARM, BANKCALL, GOTOPOOH, INTPRET, TC SUBEXIT's jump
 * to E 1470, or POODOO1 (alarm 1302). */
int lm_p33_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_P33_STOPPED 1u
#define LM_P33_REFUSED 2u
unsigned lm_p33_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_p33_digest(apollo_overlay_state *,uint32_t n);
#endif
