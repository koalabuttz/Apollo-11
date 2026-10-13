/* SPDX-License-Identifier: MIT */
#ifndef LM_R31_CASES_H
#define LM_R31_CASES_H
#include "lm_r31_routines.h"
/* Deterministic sampled entries for the Luminary099 R31 transfer routines.
 * Case n enters entry n%42 over poisoned erasable debris: the 24
 * interpretive entries at NEWOPS, then the eighteen basic entries at Z (see
 * include/lm_r31_routines.h). Debris covers any vectors, times, matrices
 * and flags (AVEGFLAG, SURFFLAG, MUNFLAG, MOONTHIS, EXTVBACT), every VAC,
 * PUSHLOC, MODE, IRQ and allowed SUPERBNK (bank 40's SUPERBNK 4 for its
 * four entries), and EB (E7 three times in four). Every case runs through the
 * trampoline with this file's routines to the first point outside them; the
 * oracle stops at the same point (lm_r31_stop). */
#define LM_R31_ROUTINES 42u
#define LM_R31_INTERPRETIVE 24u
#define LM_R31_PER_ROUTINE 512u
#define LM_R31_CASES (LM_R31_ROUTINES*LM_R31_PER_ROUTINE)
#define LM_R31_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_r31_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 when the original at (Z, FB, Q, LOC) has left the native routines: a
 * NEWOPS at a split point (3240, 3246, 3355) or outside bank 37's 3132-3154, 3166-3315, 3331-3343, 3347-3367,
 * 3371-3376 and 3404-3406, LOADTIME, ARCCOS, FINDVAC, FIXDELAY, NOVAC,
 * TASKOVER, BANKCALL, B5OFF, PRIOCHNG, ENDEXT, INTPRET, POODOO1 (alarm
 * 1302), or UNIT's 1302 exit (00,3402). */
int lm_r31_stop(agc_word z,agc_word fb,agc_word q,agc_word loc);
#define LM_R31_STOPPED 1u
#define LM_R31_REFUSED 2u
unsigned lm_r31_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_r31_digest(apollo_overlay_state *,uint32_t n);
#endif
