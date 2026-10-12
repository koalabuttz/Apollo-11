/* SPDX-License-Identifier: MIT */
#ifndef LM_P76_CASES_H
#define LM_P76_CASES_H
#include "lm_p76_routines.h"
/* Deterministic sampled entries for the P76 transfer routines. Case n enters
 * entry n%28 over poisoned erasable debris: the twelve interpretive entries
 * at NEWOPS, then the sixteen basic entries at Z (see
 * include/lm_p76_routines.h). Debris covers any vectors, times and flags
 * (CMOONFLG), every VAC, PUSHLOC, MODE, IRQ and allowed SUPERBNK, and EB
 * (EBANK=TIG three times in four); QPRET is P76SUB1's real return (13,2263)
 * three times in four, else a CADR in another bank. Every case runs through the
 * trampoline with this file's routines to the first point outside them; the
 * oracle stops at the same point (lm_p76_stop). */
#define LM_P76_ROUTINES 28u
#define LM_P76_INTERPRETIVE 12u
#define LM_P76_PER_ROUTINE 512u
#define LM_P76_CASES (LM_P76_ROUTINES*LM_P76_PER_ROUTINE)
#define LM_P76_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_p76_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 13,2212-2350's interpretive code, UPFLAG, PHASCHNG, BANKCALL, GOTOPOOH,
 * INTPRET, or POODOO1 (alarm 1302). */
int lm_p76_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_P76_STOPPED 1u
#define LM_P76_REFUSED 2u
unsigned lm_p76_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_p76_digest(apollo_overlay_state *,uint32_t n);
#endif
