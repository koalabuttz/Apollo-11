/* SPDX-License-Identifier: MIT */
#ifndef LM_P30_CASES_H
#define LM_P30_CASES_H
#include "lm_p30_routines.h"
/* Deterministic sampled entries for the P30 and S30.1 transfer routines.
 * Case n enters entry n%18 over poisoned erasable debris: the ten
 * interpretive entries at NEWOPS (seven in bank 34, three in bank 35), then
 * the eight basic entries at Z in bank 35 (see include/lm_p30_routines.h).
 * Debris covers any vectors, times and flags, every VAC, PUSHLOC, MODE, IRQ
 * and allowed SUPERBNK, and EB (E7 three times in four); QPRET and QTEMP
 * are P30's real return from S30.1 (35,2015) three times in four, else a
 * CADR in banks 2-27. Every case runs through the trampoline with this
 * file's routines to the first point outside them; the oracle stops at the
 * same point (lm_p30_stop). */
#define LM_P30_ROUTINES 18u
#define LM_P30_INTERPRETIVE 10u
#define LM_P30_PER_ROUTINE 512u
#define LM_P30_CASES (LM_P30_ROUTINES*LM_P30_PER_ROUTINE)
#define LM_P30_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_p30_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 34,2000-2056, 35,2013-2016 and 35,2022-2024, UPFLAG,
 * DOWNFLAG, VNPOOH, INTPRET, or POODOO1 (alarm 1302). */
int lm_p30_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_P30_STOPPED 1u
#define LM_P30_REFUSED 2u
unsigned lm_p30_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_p30_digest(apollo_overlay_state *,uint32_t n);
#endif
