/* SPDX-License-Identifier: MIT */
#ifndef LM_S3435_CASES_H
#define LM_S3435_CASES_H
#include "lm_s3435_routines.h"
/* Deterministic sampled entries for the S34/35 transfer routines. Case n
 * enters routine n%4 (S34/35.1, NOVRWRT, S34/35.4, LOMAT) at its NEWOPS over
 * poisoned erasable debris: any vectors (one state in eight with the vector
 * UNIT reads zero or nearly so), every VAC, PUSHLOC, MODE, IRQ and allowed
 * SUPERBNK, and QPRET (or NORMEX for NOVRWRT) one of the routine's real
 * callers' returns or another bank's CADR. Every case runs through the
 * trampoline with this file's routines to the first point outside them;
 * the oracle stops at the same points: a NEWOPS outside the routines or
 * UNIT's POODOO1 (alarm 1302). */
#define LM_S3435_PER_ROUTINE 4096u
#define LM_S3435_CASES (LM_S3435_ROUTINES*LM_S3435_PER_ROUTINE)
#define LM_S3435_FETCH ((uint16_t)(2048u+(06070u-04000u)))
void lm_s3435_input(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* 1 if (FB, LOC) at NEWOPS lies in this file's routines (the oracle runs
 * through those NEWOPS). */
int lm_s3435_owned_newops(agc_word fb,agc_word loc);
/* Native run of case n through the program of the routines: 1 stopped at an
 * unmapped point, 2 refused, 3 any other outcome. */
#define LM_S3435_STOPPED 1u
#define LM_S3435_REFUSED 2u
unsigned lm_s3435_run(apollo_overlay_state *,p66_bias_io *,uint32_t n);
/* Two 16-bit running sums over the tag (1 stopped, 0 refused, 3 other), the
 * 2048 erasable words (A and L 16-bit) and the interrupt-enable state. */
uint32_t lm_s3435_digest(apollo_overlay_state *,uint32_t n);
#endif
