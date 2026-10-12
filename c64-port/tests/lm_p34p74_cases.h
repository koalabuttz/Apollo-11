/* SPDX-License-Identifier: MIT */
#ifndef LM_P34P74_CASES_H
#define LM_P34P74_CASES_H
#include "lm_p34p74_routines.h"
/* Deterministic sampled entries for the P34/P74 transfer routines. Case n
 * enters entry n%34 over poisoned erasable debris: the sixteen interpretive
 * entries at NEWOPS, then the eighteen basic entries at Z (see
 * include/lm_p34p74_routines.h). Debris covers any times, vectors and flags
 * (ETPIFLAG, ITSWICH), ELEV zero in one P34 start in two and MPAC zero in
 * one S33/34.1 return in two (both BZE outcomes), every VAC, PUSHLOC, MODE,
 * IRQ and allowed SUPERBNK, and EB (EBANK=SUBEXIT three times in four).
 * Every case runs through the trampoline with this file's routines to the
 * first point outside them; the oracle stops at the same point
 * (lm_p34p74_stop). */
#define LM_P34P74_ROUTINES 34u
#define LM_P34P74_INTERPRETIVE 16u
#define LM_P34P74_PER_ROUTINE 512u
#define LM_P34P74_CASES (LM_P34P74_ROUTINES*LM_P34P74_PER_ROUTINE)
#define LM_P34P74_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_p34p74_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 35,2562-2724, AVFLAGA, AVFLAGP, P20FLGON, DISPLAYE,
 * VNPOOH, ALARM, BANKCALL, GOTOPOOH, INTPRET, or POODOO1 (alarm 1302). */
int lm_p34p74_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_P34P74_STOPPED 1u
#define LM_P34P74_REFUSED 2u
unsigned lm_p34p74_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_p34p74_digest(apollo_overlay_state *,uint32_t n);
#endif
