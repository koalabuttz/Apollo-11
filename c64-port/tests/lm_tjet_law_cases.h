/* SPDX-License-Identifier: MIT */
#ifndef LM_TJET_LAW_CASES_H
#define LM_TJET_LAW_CASES_H
#include "lm_tjet_law.h"
/* lm-tjet-law cases: case n enters TJETLAW for axis n % 3 (AXISCTR -1, 0,
 * +1: P, U, V) from a random image with FB bank 17 and EB 6, A and L random,
 * and a caller return in Q (SWRETURN, a fixed-fixed address or a bank-17
 * address outside the file), and runs through the trampoline to the
 * caller's return. Cohorts put the attitude and rate errors and the 1/ACCS
 * parameters in the fine-law ranges, at the rough-law edges, or at +0/-0.
 * tools/lm_tjet_law_reference.c runs the same entry in the original
 * program. */
#define LM_TJ_CASES 16384u
typedef struct {agc_word stop_z,stop_fb,superbank;} lm_tj_case;
void lm_tj_input(apollo_overlay_state *,lm_tj_case *,uint16_t n);
/* Runs case n natively; returns 0 at the caller's return, 1 otherwise. */
unsigned lm_tj_run(apollo_overlay_state *,uint16_t n,uint16_t *steps);
uint32_t lm_tj_digest(apollo_overlay_state *,uint16_t n);
#endif
