/* SPDX-License-Identifier: MIT */
#ifndef LM_P32_CASES_H
#define LM_P32_CASES_H
#include "lm_p32_routines.h"
/* Deterministic sampled entries for the P32/P72, ALMXITA and ALMXIT
 * transfer routines. Case n enters entry n%40 over poisoned erasable
 * debris: the nineteen interpretive entries at NEWOPS, then the twenty-one
 * basic entries at Z (see include/lm_p32_routines.h). Debris covers any
 * times (both signs of TCSI), vectors, index registers and flags
 * (FINALFLG, RVSW), every VAC, PUSHLOC, MODE, IRQ and allowed SUPERBNK, EB
 * (E7 three times in four), and an ALARM/TB entry (1-7) in CSIALRM for
 * ALMXIT and in X2 for ALMXITA. Every case runs through the trampoline with this file's
 * routines to the first point outside them; the oracle stops at the same
 * point (lm_p32_stop). */
#define LM_P32_ROUTINES 40u
#define LM_P32_INTERPRETIVE 19u
#define LM_P32_PER_ROUTINE 512u
#define LM_P32_CASES (LM_P32_ROUTINES*LM_P32_PER_ROUTINE)
#define LM_P32_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_p32_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 35,2036-2043, 2063-2111, 2113, 2125-2172 and 2176-2215,
 * AVFLAGA, AVFLAGP, P20FLGON, VNPOOH, VARALARM, BANKCALL, GOTOPOOH,
 * INTPRET, or POODOO1 (alarm 1302). */
int lm_p32_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_P32_STOPPED 1u
#define LM_P32_REFUSED 2u
unsigned lm_p32_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_p32_digest(apollo_overlay_state *,uint32_t n);
#endif
