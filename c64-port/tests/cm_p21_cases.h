/* SPDX-License-Identifier: MIT */
#ifndef CM_P21_CASES_H
#define CM_P21_CASES_H
#include "cm_p21_routines.h"
/* Deterministic sampled entries for the Comanche055 P21 transfer routines.
 * Case n enters entry n%24 over poisoned erasable debris: the ten
 * interpretive entries at NEWOPS, then the fourteen basic entries at Z (see
 * include/cm_p21_routines.h). Debris covers any vectors, times and flags
 * (P21FLAG), OPTION2, P21ORIG and X2 zero in one case in two each (both
 * BHIZ and BZE outcomes), every VAC, PUSHLOC, MODE, IRQ and allowed
 * SUPERBNK, and EB (E4 three times in four). Every case runs through the
 * trampoline with this file's routines to the first point outside them; the
 * oracle stops at the same point (cm_p21_stop). */
#define CM_P21_ROUTINES 24u
#define CM_P21_INTERPRETIVE 10u
#define CM_P21_PER_ROUTINE 512u
#define CM_P21_CASES (CM_P21_ROUTINES*CM_P21_PER_ROUTINE)
#define CM_P21_FETCH ((uint16_t)(2048u+(06037u-04000u)))
void cm_p21_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 37,2020-2043, 2044-2130 and 2137-2144, ARCSIN, BANKCALL,
 * GOTOPOOH, INTPRET, or POODOO (alarm 1302). */
int cm_p21_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define CM_P21_STOPPED 1u
#define CM_P21_REFUSED 2u
unsigned cm_p21_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t cm_p21_digest(apollo_overlay_state *,uint32_t n);
#endif
