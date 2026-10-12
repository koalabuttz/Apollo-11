/* SPDX-License-Identifier: MIT */
#ifndef LM_P35_CASES_H
#define LM_P35_CASES_H
#include "lm_p35_routines.h"
/* Deterministic sampled entries for the P35/P75 transfer routines. Case n
 * enters entry n%13 over poisoned erasable debris: the eight interpretive
 * entries (the CALL SELECTMU after INTPRET, P35/P75B, LOADTIME's, PRECSET's,
 * S34/35.1's, S34/35.2's, S34/35.5's and VN1645's returns) at NEWOPS, then
 * the five basic entries (P35, AVFLAGA's return, P75, AVFLAGP's return and
 * P20FLGON's return) at Z. Debris covers any times and flags, every VAC,
 * PUSHLOC, MODE, IRQ and allowed SUPERBNK, and EB (EBANK=KT three times in
 * four). Every case runs through the trampoline with this file's routines
 * to the first point outside them; the oracle stops at the same point
 * (lm_p35_stop). */
#define LM_P35_ROUTINES 13u
#define LM_P35_INTERPRETIVE 8u
#define LM_P35_PER_ROUTINE 1024u
#define LM_P35_CASES (LM_P35_ROUTINES*LM_P35_PER_ROUTINE)
#define LM_P35_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_p35_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, LOC) has left the native routines: a NEWOPS
 * outside 35,2737-2763 (SELECTMU, PRECSET, S34/35.1, S34/35.2, S34/35.5,
 * VN1645), AVFLAGA, AVFLAGP, P20FLGON, INTPRET or LOADTIME (RTB). */
int lm_p35_stop(agc_word z,agc_word fb,agc_word loc);
#define LM_P35_STOPPED 1u
#define LM_P35_REFUSED 2u
unsigned lm_p35_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_p35_digest(apollo_overlay_state *,uint32_t n);
#endif
