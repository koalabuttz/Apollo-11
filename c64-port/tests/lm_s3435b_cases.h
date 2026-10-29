/* SPDX-License-Identifier: MIT */
#ifndef LM_S3435B_CASES_H
#define LM_S3435B_CASES_H
#include "lm_s3435b_routines.h"
#include "lm_s3435_routines.h"
/* Deterministic sampled entries for the S34/35.3 and GOINT/INTINT transfer
 * routines. Case n enters routine n%7 (S34/35.3, its LOMAT and INTINT
 * returns, GOINT, INTINT, and INTINT's INTSTALL and INTEGRVS returns) at
 * its NEWOPS over poisoned erasable debris: any vectors, matrix and flags,
 * every VAC, PUSHLOC (as far as the routine's pushes leave room), MODE, IRQ and allowed SUPERBNK, and QPRET, NORMEX and
 * RTRN each one of the real callers' returns or another bank's CADR. Every
 * case runs through the trampoline with this file's routines and the
 * lm-s3435 routines (LOMAT, NOVRWRT) to the first point outside them; the
 * oracle stops at the same NEWOPS (INTSTALL, INTEGRVS, a caller's return)
 * or at UNIT's POODOO1. */
#define LM_S3435B_PER_ROUTINE 2048u
#define LM_S3435B_CASES (LM_S3435B_ROUTINES*LM_S3435B_PER_ROUTINE)
#define LM_S3435B_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_s3435b_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
int lm_s3435b_owned_newops(agc_word fb,agc_word loc);
#define LM_S3435B_STOPPED 1u
#define LM_S3435B_REFUSED 2u
unsigned lm_s3435b_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
uint32_t lm_s3435b_digest(apollo_overlay_state *,uint32_t n);
#endif
