/* SPDX-License-Identifier: MIT */
#ifndef CM_P76_CASES_H
#define CM_P76_CASES_H
#include "cm_p76_routines.h"
/* Deterministic sampled entries for the Comanche055 P76 transfer routines.
 * Case n enters entry n%27 over poisoned erasable debris: the twelve
 * interpretive entries at NEWOPS, then the fifteen basic entries at Z (see
 * include/cm_p76_routines.h). Debris covers any vectors, times and flags,
 * X2 zero in one case in two (both BHIZ outcomes), every VAC, PUSHLOC,
 * MODE, IRQ and allowed SUPERBNK, and EB (EBANK=TIG three times in four);
 * QPRET is one of P76SUB1's real returns (13,2140 or 13,2163) three times
 * in four, else a CADR in another bank. Every case runs through the
 * trampoline with this file's routines to the first point outside them; the
 * oracle stops at the same point (cm_p76_stop). */
#define CM_P76_ROUTINES 27u
#define CM_P76_INTERPRETIVE 12u
#define CM_P76_PER_ROUTINE 512u
#define CM_P76_CASES (CM_P76_ROUTINES*CM_P76_PER_ROUTINE)
#define CM_P76_FETCH ((uint16_t)(2048u+(06037u-04000u)))
void cm_p76_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS outside 13,2110-2174, 2202-2206, 2211 and 2222-2230, UPFLAG,
 * PHASCHNG, BANKCALL, GOTOPOOH, INTPRET, or POODOO (alarm 1302). */
int cm_p76_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define CM_P76_STOPPED 1u
#define CM_P76_REFUSED 2u
unsigned cm_p76_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t cm_p76_digest(apollo_overlay_state *,uint32_t n);
#endif
